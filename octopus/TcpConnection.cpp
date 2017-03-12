#include "TcpConnection.h"
#include "Log.h"

namespace Octopus {

	namespace Core {

		TcpConnection::TcpConnection(Reactor* pReactor,
			const std::string& name,
			int sockfd,
			const SocketAddress& localAddr,
			const SocketAddress& peerAddr)
			:mpReactor(pReactor)
			, mName(name)
			, mSocket(new Socket(sockfd))
			, mEventHandler(new EventHandler(pReactor, sockfd))
			, mLocalAddress(localAddr)
			, mRemoteAddress(peerAddr)
			, mConnectionState(eConnecting)
			, mReading(false)
			, mHightWaterMark(64 * 1024 * 1024)//64k
		{
			mEventHandler->setReadCallback(
				std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
			mEventHandler->setWriteCallback(
				std::bind(&TcpConnection::handleWrite, this));
			mEventHandler->setCloseCallback(
				std::bind(&TcpConnection::handleClose, this));
			mEventHandler->setErrorCallback(
				std::bind(&TcpConnection::handleError, this));

			LOG_DEBUG << "TcpConnection::ctor[" << mName << "] at " << this
				<< " fd=" << sockfd;

			mSocket->setKeepAlive(true);
		}


		TcpConnection::~TcpConnection()
		{
		}

		void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)
		{
			mConnectionCallback = cb;
		}

		void TcpConnection::setMessageCallback(const MessageCallback& cb)
		{
			mMessageCallback = cb;
		}

		void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb)
		{
			mWriteCompleteCallback = cb;
		}

		void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
		{
			mHighWaterMarkCallback = cb;
			mHightWaterMark = highWaterMark;
		}

		void TcpConnection::setCloseCallback(const CloseCallback& cb)
		{
			mCloseCallback = cb;
		}

		void TcpConnection::handleRead(Timestamp receiveTime)
		{
			int savedErrno = 0;
			ssize_t n = mReadBuffer.readFd(mEventHandler->fd(), &savedErrno);
			if (n > 0)
			{
				mMessageCallback(shared_from_this(), &mReadBuffer, receiveTime);
			}
			else if (n == 0)
			{
				handleClose();
			}
			else
			{
				errno = savedErrno;
				LOG_SYSERR << "TcpConnection::handleRead";
				handleError();
			}
		}
		void TcpConnection::handleWrite()
		{
			if (mEventHandler->isWriting())
			{
				ssize_t n = ::write(mEventHandler->fd(),
					mWriteBuffer.peek(),
					mWriteBuffer.availReadBytes());
				if (n > 0)
				{
					mWriteBuffer.recycle(n);
					if (mWriteBuffer.availReadBytes() == 0)
					{
						mEventHandler->enableWrite(false);
						if (mWriteCompleteCallback)
						{
							//这里可以直接调用，因为只有一个线程
							mpReactor->queuePending(std::bind(mWriteCompleteCallback, shared_from_this()));
						}
						if (mConnectionState == eDisconnecting)
						{
							shutdownInReactor();
						}
					}
				}
				else
				{
					LOG_SYSERR << "TcpConnection::handleWrite";

				}
			}
			else
			{
				LOG_TRACE << "Connection fd = " << mEventHandler->fd()
					<< " is down, no more writing";
			}
		}
		void TcpConnection::handleClose()
		{

			LOG_TRACE << "fd = " << mEventHandler->fd() << " handle close " ;
			assert(mConnectionState == eConnected || mConnectionState == eDisconnecting);

			// we don't close fd, leave it to dtor, so we can find leaks easily.
			mConnectionState = eDisconnected;

			mEventHandler->disableAll();

			TcpConnectionPtr guardThis(shared_from_this());
			mConnectionCallback(guardThis);
			// must be the last line
			mCloseCallback(guardThis);
		}
		void TcpConnection::handleError()
		{
			int err;

			int optval;
			socklen_t optlen = static_cast<socklen_t>(sizeof optval);

			if (::getsockopt(mEventHandler->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
			{
				err = errno;
			}
			else
			{
				err = optval;
			}

			LOG_ERROR << "TcpConnection::handleError [" << mName
				<< "] - SO_ERROR = " << err << " ";
		}

		void TcpConnection::shutdownInReactor()
		{
			if (!mEventHandler->isWriting())
			{
				// we are not writing
				mSocket->shutdownWrite();
			}
		}

		void TcpConnection::shutdown()
		{
			
			if (mConnectionState == eConnected)
			{
				mConnectionState = eDisconnecting;
				//这里可以直接执行，只有一个线程
				mpReactor->queuePending(std::bind(&TcpConnection::shutdownInReactor, this));
			}
		}

	}
}
