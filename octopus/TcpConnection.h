#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "Base.h"
#include "SocketAddress.h"
#include "Reactor.h"
#include "Socket.h"
#include "EventHandler.h"
#include "Buffer.h"

#include <errno.h>

namespace Octopus {

	namespace Core {

		class TcpConnection;

		typedef std::shared_ptr<TcpConnection>               TcpConnectionPtr;
		typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
		typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
		typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
		typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
     	typedef std::function<void(const TcpConnectionPtr&,	Buffer*, Timestamp)> MessageCallback;

		class TcpConnection : public Noncopyable, std::enable_shared_from_this<TcpConnection>
		{
		public:

			enum ECONNECT_STATE
			{
				eConnecting = 0,
				eConnected,
				eDisconnecting,
				eDisconnected,
			};

			TcpConnection(Reactor* pReactor,
				const std::string& name,
				int sockfd,
				const SocketAddress& localAddr,
				const SocketAddress& peerAddr);
			virtual ~TcpConnection();

			Reactor*           getReactor();
			const std::string& getName();
			const SocketAddress& getLocalAddress();
			const SocketAddress& getRemoteAddress();

			bool  isConnected();
			bool  isDisconnected();

			void  send(const void* message, int len);
			void  send(Buffer* message);


			void setConnectionCallback(const ConnectionCallback& cb);
			void setMessageCallback(const MessageCallback& cb);
			void setWriteCompleteCallback(const WriteCompleteCallback& cb);
			void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark);
			void setCloseCallback(const CloseCallback& cb);

			void handleRead(Timestamp receiveTime);
			void handleWrite();
			void handleClose();
			void handleError();

			void shutdown();
			void forceCloseInReactor();
			void forceClose();
			void forceCloseWithDelay(double seconds);

			void setTcpNoDelay(bool on);

			void startRead();
			void stopRead();
			void startReadInReactor();
			void stopReadInReactor();
			bool isReading();

			Buffer* getReadBuffer();
			Buffer* getWriteBuffer();

			void connectionEstablished();
			void connectionDestroyed();
		protected:
			void shutdownInReactor();
			void sendInReactor(const void* message, size_t len);
		private:

			Reactor*          mpReactor;
			const std::string mName;

			std::unique_ptr<Socket>       mSocket;
			std::unique_ptr<EventHandler> mEventHandler;

			const SocketAddress  mLocalAddress;
			const SocketAddress  mRemoteAddress;

			ECONNECT_STATE     mConnectionState;

			bool               mReading;

			Buffer             mReadBuffer;
			Buffer             mWriteBuffer;

			std::size_t        mHightWaterMark;

			ConnectionCallback    mConnectionCallback;
			MessageCallback       mMessageCallback;
			WriteCompleteCallback mWriteCompleteCallback;
			HighWaterMarkCallback mHighWaterMarkCallback;
			CloseCallback         mCloseCallback;
		};

	}
}

#endif

