#include "Socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  
#include <stdio.h>  
#include <unistd.h>

#include <sys/types.h>

#include <sys/socket.h>

#include "Log.h"
#include "SocketAddress.h"

namespace Octopus {

	namespace Core {

		Socket::Socket(int fd) :mfd(fd)
		{
		}


		Socket::~Socket()
		{
			if (::close(mfd) < 0)
			{
				LOG_SYSERR << "close socket";
			}
		}

		void Socket::bind(const SocketAddress& address)
		{
			if (::bind(mfd, address.getSockAddr(), sizeof(struct sockaddr_in6)) < 0)
			{
				LOG_SYSERR << "bind socket";
			}
		}

		void Socket::listen()
		{			
			if (::listen(mfd, SOMAXCONN) < 0)
			{
				LOG_SYSFATAL << "socket listen";
			}
		}

		int Socket::accept(SocketAddress& peeraddr)
		{
			struct sockaddr_in6 addr;
			bzero(&addr, sizeof(addr));		

			socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
#if VALGRIND || defined (NO_ACCEPT4)
			int connfd = ::accept(mfd, sockaddr_cast(addr), &addrlen);
			setNonBlockAndCloseOnExec(connfd);
#else
			int connfd = ::accept4(mfd, sockaddr_cast(&addr),
				&addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
			if (connfd < 0)
			{
				int savedErrno = errno;
				LOG_SYSERR << "socket accept";
				switch (savedErrno)
				{
				case EAGAIN:
				case ECONNABORTED:
				case EINTR:
				case EPROTO: // ???
				case EPERM:
				case EMFILE: // per-process lmit of open file desctiptor ???
							 // expected errors
					errno = savedErrno;
					break;
				case EBADF:
				case EFAULT:
				case EINVAL:
				case ENFILE:
				case ENOBUFS:
				case ENOMEM:
				case ENOTSOCK:
				case EOPNOTSUPP:
					// unexpected errors
					LOG_FATAL << "unexpected error of ::accept " << savedErrno;
					break;
				default:
					LOG_FATAL << "unknown error of ::accept " << savedErrno;
					break;
				}
			}
			

			if (connfd >= 0)
			{
				peeraddr.setSockAddrInet6(addr);
			}
			return connfd;
		}

		void Socket::shutdownWrite()
		{
			if (::shutdown(mfd, SHUT_WR) < 0)
			{
				LOG_SYSERR << "shutdown write";
			}
		}

		void Socket::setTcpNoDelay(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(mfd, IPPROTO_TCP, TCP_NODELAY,
				&optval, static_cast<socklen_t>(sizeof optval));
			
		}

		void Socket::setReuseAddr(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(mfd, SOL_SOCKET, SO_REUSEADDR,
				&optval, static_cast<socklen_t>(sizeof optval));
			
		}

		void Socket::setReusePort(bool on)
		{
#ifdef SO_REUSEPORT
			int optval = on ? 1 : 0;
			int ret = ::setsockopt(mfd, SOL_SOCKET, SO_REUSEPORT,
				&optval, static_cast<socklen_t>(sizeof optval));
			if (ret < 0 && on)
			{
				LOG_SYSERR << "SO_REUSEPORT failed.";
			}
#else
			if (on)
			{
				LOG_ERROR << "SO_REUSEPORT is not supported.";
			}
#endif
		}

		void Socket::setKeepAlive(bool on)
		{
			int optval = on ? 1 : 0;
			::setsockopt(mfd, SOL_SOCKET, SO_KEEPALIVE,
				&optval, static_cast<socklen_t>(sizeof optval));
			// FIXME CHECK
		}

		const struct sockaddr* Socket::sockaddr_cast(const struct sockaddr_in6* addr)
		{
			return static_cast<const struct sockaddr*>((const void*)(addr));
		}

		struct sockaddr* Socket::sockaddr_cast(struct sockaddr_in6* addr)
		{
			return static_cast<struct sockaddr*>((void*)(addr));
		}


	}
}
