#include "TcpConnection.h"
#include "Log.h"

namespace Octopus {

	namespace Core {

		template<typename T>
		class WeakCallback
		{
		public:
			WeakCallback(const std::weak_ptr<T>& weakPtr, const std::function<void(T*)>& function)
				:mWeakPtr(weakPtr), mCallback(function)
			{}

			void operator()()const
			{
				std::shared_ptr<T> sp(mWeakPtr.lock());
				if (sp)
				{
					mCallback(sp.get());
				}
			}
		private:

			std::weak_ptr<T> mWeakPtr;
			std::function<void(T*)> mCallback;
		};

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
				mpReactor->runInReactor(std::bind(&TcpConnection::shutdownInReactor, this));
			}
		}

		Reactor* TcpConnection::getReactor()
		{
			return mpReactor;
		}

		const std::string& TcpConnection::getName()
		{
			return mName;
		}

		const SocketAddress& TcpConnection::getLocalAddress()
		{
			return mLocalAddress;
		}
		const SocketAddress& TcpConnection::getRemoteAddress()
		{
			return mRemoteAddress;
		}
		bool  TcpConnection::isConnected()
		{
			return mConnectionState == eConnected;
		}
		bool  TcpConnection::isDisconnected()
		{
			return mConnectionState == eDisconnected;
		}

		void  TcpConnection::send(const void* message, int len)
		{
			sendInReactor(message, len);
		}
		void  TcpConnection::send(Buffer* buf)
		{
			if (mConnectionState == eConnected)
			{
				sendInReactor(buf->peek(), buf->availReadBytes());
				buf->retrieveAll();
			}
		}
		void TcpConnection::sendInReactor(const void* data, size_t len)
		{

			ssize_t nwrote = 0;
			size_t remaining = len;
			bool faultError = false;
			if (mConnectionState == eDisconnected)
			{
				LOG_WARN << "disconnected, give up writing";
				return;
			}
			// mWriteBuffer中没有可发送数据，直接发送
			if (!mEventHandler->isWriting() && mWriteBuffer.availReadBytes() == 0)
			{
				nwrote = ::write(mEventHandler->fd(), data, len);
				if (nwrote >= 0)
				{
					remaining = len - nwrote;
					if (remaining == 0 && mWriteCompleteCallback)
					{
						mpReactor->runInReactor(std::bind(mWriteCompleteCallback, shared_from_this()));
					}
				}
				else // nwrote < 0
				{
					nwrote = 0;
					if (errno != EWOULDBLOCK)
					{
						LOG_SYSERR << "TcpConnection::sendInLoop";
						if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
						{
							faultError = true;
						}
					}
				}
			}

			assert(remaining <= len);
			if (!faultError && remaining > 0)
			{
				//如果发送数据过多，调用超量回调函数
				size_t oldLen = mWriteBuffer.availReadBytes();
				if (oldLen + remaining >= mHightWaterMark
					&& oldLen < mHightWaterMark
					&& mHighWaterMarkCallback)
				{
					mpReactor->queuePending(std::bind(mHighWaterMarkCallback, shared_from_this(), oldLen + remaining));
				}
				//把多余数据放到发送buffer中
				mWriteBuffer.append(static_cast<const char*>(data) + nwrote, remaining);
				if (!mEventHandler->isWriting())
				{
					mEventHandler->enableWrite(true);
				}
			}
		}

		void TcpConnection::forceCloseInReactor()
		{
			if (mConnectionState == eConnected || mConnectionState == eConnecting)
			{				
				handleClose();
			}
		}

		void TcpConnection::forceClose()
		{
			if (mConnectionState == eConnected || mConnectionState == eConnecting)
			{
				mConnectionState = eDisconnecting;
				mpReactor->queuePending(std::bind(&TcpConnection::forceCloseInReactor, shared_from_this()));
			}
		}
		void TcpConnection::forceCloseWithDelay(double seconds)
		{
			if (mConnectionState == eConnected || mConnectionState == eConnecting)
			{
				mConnectionState = eDisconnecting;

				//可能已经close，所以用weakptr
				mpReactor->setDelayTimer(
					seconds,
					WeakCallback<TcpConnection>(shared_from_this(),
						&TcpConnection::forceClose)); 
			}
		}
		void  TcpConnection::setTcpNoDelay(bool on)
		{
			mSocket->setTcpNoDelay(on);
		}
		void  TcpConnection::startRead()
		{
			mpReactor->runInReactor(std::bind(&TcpConnection::startReadInReactor, this));
		}
		void  TcpConnection::stopRead()
		{
			mpReactor->runInReactor(std::bind(&TcpConnection::stopReadInReactor, this));
		}
		void TcpConnection::startReadInReactor()
		{
			//设置读，向poll中添加读event
			if (!mReading || !mEventHandler->isReading())
			{
				mEventHandler->enableRead(true);
				mReading = true;
			}
		}
		void TcpConnection::stopReadInReactor()
		{
			//取消读，在poll中取消读event
			if (mReading || mEventHandler->isReading())
			{
				mEventHandler->enableRead(false);
				mReading = false;
			}
		}
		bool  TcpConnection::isReading()
		{
			return mReading;
		}
		Buffer* TcpConnection::getReadBuffer()
		{
			return &mReadBuffer;
		}
		Buffer* TcpConnection::getWriteBuffer()
		{
			return &mWriteBuffer;
		}

		void TcpConnection::connectionEstablished()
		{
			
			assert(mConnectionState == eConnecting);
			mConnectionState = eConnected;
			
			mEventHandler->bindOwner(shared_from_this());
			mEventHandler->enableRead(true);

			mConnectionCallback(shared_from_this());
		}

		void TcpConnection::connectionDestroyed()
		{
			
			if (mConnectionState == eConnected)
			{
				mConnectionState = eDisconnected;
				mEventHandler->disableAll();

				mConnectionCallback(shared_from_this());
			}
			mEventHandler->remove();
		}

	}
}
