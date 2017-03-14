#include "Connector.h"
#include "Log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h> 
#include <strings.h> 
#include <sys/socket.h>
#include <unistd.h>

namespace Octopus {

	namespace Core {

		static const int MAX_RETRY_DELAY = 30000;
		static const int INIT_RETRY_DELAY = 500;

		static struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr)
		{
			return static_cast<struct sockaddr*>(static_cast<void*>(addr));
		}

		static struct sockaddr_in6 getLocalAddr(int sockfd)
		{
			struct sockaddr_in6 localaddr;
			bzero(&localaddr, sizeof localaddr);
			socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
			if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
			{
				LOG_SYSERR << "socket::getLocalAddr";
			}
			return localaddr;
		}

		static struct sockaddr_in6 getPeerAddr(int sockfd)
		{
			struct sockaddr_in6 peeraddr;
			bzero(&peeraddr, sizeof peeraddr);
			socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
			if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
			{
				LOG_SYSERR << "socket::getPeerAddr";
			}
			return peeraddr;
		}

#if !(__GNUC_PREREQ (4,6))
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
		static bool isSelfConnect(int sockfd)
		{
			struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
			struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
			if (localaddr.sin6_family == AF_INET)
			{
				const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
				const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
				return laddr4->sin_port == raddr4->sin_port
					&& laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
			}
			else if (localaddr.sin6_family == AF_INET6)
			{
				return localaddr.sin6_port == peeraddr.sin6_port
					&& memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
			}
			else
			{
				return false;
			}
		}

		Connector::Connector(Reactor* pReactor, const SocketAddress& address)
			:mpReactor(pReactor)
			, mAddress(address)
			, mConnect(false)
			, mConnectState(eDisconnected)
			, mretryDelayMs(INIT_RETRY_DELAY)
		{
		}


		Connector::~Connector()
		{
		}

		void Connector::setConnectionCallback(const ConnectionCallback& cb)
		{
			mConnectionCallback = cb;
		}

		const SocketAddress&  Connector::getServerAddress()
		{
			return mAddress;
		}

		void Connector::retry(int sockfd)
		{
			if (::close(sockfd) < 0)
			{
				LOG_SYSERR << "socket::close";
			}

			mConnectState = eDisconnected;
			
			if (mConnect)
			{
				LOG_INFO << "Connector::retry - Retry connecting to " << mAddress.hostAndPort()
					<< " in " << mretryDelayMs << " milliseconds. ";

				mpReactor->setDelayTimer(mretryDelayMs / 1000.0,
					std::bind(&Connector::startInReactor, shared_from_this()));

				mretryDelayMs = std::min(mretryDelayMs * 2, MAX_RETRY_DELAY);
			}
			else
			{
				LOG_DEBUG << "do not connect";
			}
		}

		void Connector::bindEventHandler(int sockfd)
		{
			//把句柄与EventHandler关联
			mConnectState = eConnecting;
			
			mEventHandler.reset(new EventHandler(mpReactor, sockfd));
			
			mEventHandler->setWriteCallback(
				std::bind(&Connector::handleWrite, this)); 
			mEventHandler->setErrorCallback(
				std::bind(&Connector::handleError, this)); 															
															
			mEventHandler->enableWrite(true);
		}

		void Connector::connect()
		{
			int sockfd = ::socket(mAddress.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
			if (sockfd < 0)
			{
				//这个log中会调用exit函数，程序结束
				LOG_SYSFATAL << "create socket failed, Connector::connect()";				
			}

			int ret = ::connect(sockfd, mAddress.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in6)));

			int savedErrno = (ret == 0) ? 0 : errno;
			switch (savedErrno)
			{
			case 0:
			case EINPROGRESS:
			case EINTR:
			case EISCONN:
				bindEventHandler(sockfd);
				break;

			case EAGAIN:
			case EADDRINUSE:
			case EADDRNOTAVAIL:
			case ECONNREFUSED:
			case ENETUNREACH:
				retry(sockfd);
				break;

			case EACCES:
			case EPERM:
			case EAFNOSUPPORT:
			case EALREADY:
			case EBADF:
			case EFAULT:
			case ENOTSOCK:
				LOG_SYSERR << "connect error in Connector::connect " << savedErrno;
				if (::close(sockfd) < 0)
				{
					LOG_SYSERR << "socket::close";
				}
				break;

			default:
				LOG_SYSERR << "Unexpected error in Connector::connect " << savedErrno;
				if (::close(sockfd) < 0)
				{
					LOG_SYSERR << "socket::close";
				}
				
				break;
			}
		}

		void Connector::resetEventHandler()
		{
			mEventHandler.reset();
		}

		int Connector::unbindEventHandler()
		{
			//解除与eventhandler的绑定
			mEventHandler->disableAll();
			mEventHandler->remove();
			int sockfd = mEventHandler->fd();
			
			mpReactor->queuePending(std::bind(&Connector::resetEventHandler, this));

			return sockfd;
		}

		void Connector::startInReactor()
		{
			//start()之后调用
			assert(mConnectState == eDisconnected);
			if (mConnect)
			{
				connect();
			}
			else
			{
				LOG_DEBUG << "do not connect";
			}
		}

		void Connector::stopInReactor()
		{
			//stop()之后调用
			if (mConnectState == eConnecting)
			{
				mConnectState = eDisconnected;
				int sockfd = unbindEventHandler();
				retry(sockfd);
			}
		}

		void Connector::start()
		{
			mConnect = true;
			//
			mpReactor->runInReactor(std::bind(&Connector::startInReactor, this)); 
		}

		void Connector::stop()
		{
			mConnect = false;
			mpReactor->queuePending(std::bind(&Connector::stopInReactor, this));
																		 
		}

		void Connector::handleWrite()
		{
			LOG_TRACE << "Connector::handleWrite " << mConnectState;

			if (mConnectState == eConnecting)
			{
				int sockfd = unbindEventHandler();

				int err ;
				int optval;

				socklen_t optlen = static_cast<socklen_t>(sizeof optval);

				if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
				{
					err = errno;
				}
				else
				{
					err = optval;
				}
				if (err)
				{
					LOG_WARN << "Connector::handleWrite - SO_ERROR = "
						<< err << " ";
					retry(sockfd);
				}
				else if (isSelfConnect(sockfd))
				{
					LOG_WARN << "Connector::handleWrite - Self connect";
					retry(sockfd);
				}
				else
				{
					
					mConnectState = eConnected;
					if (mConnect)
					{
						mConnectionCallback(sockfd);
					}
					else
					{
						if (::close(sockfd) < 0)
						{
							LOG_SYSERR << "sockets::close";
						}
					}
				}
			}
			else
			{
				
				assert(mConnectState == eDisconnected);
			}
		}

		void Connector::handleError()
		{
			LOG_ERROR << "Connector::handleError state=" << mConnectState;
			if (mConnectState == eConnecting)
			{
				int sockfd = unbindEventHandler();
				int err; 
				int optval;
				socklen_t optlen = static_cast<socklen_t>(sizeof optval);

				if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
				{
					err = errno;
				}
				else
				{
					err = optval;
				}

				LOG_TRACE << "SO_ERROR = " << err << " ";

				retry(sockfd);
			}
		}

		void Connector::restart()
		{
			mConnectState = eDisconnected;
			
			mretryDelayMs = INIT_RETRY_DELAY;
			mConnect = true;
			startInReactor();
		}
	}
}