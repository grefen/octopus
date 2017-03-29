#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "Base.h"
#include "SocketAddress.h"
#include "Reactor.h"
#include "Socket.h"
#include "EventHandler.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Connector.h"


namespace Octopus {

	namespace Core {

		typedef std::shared_ptr<Connector> ConnectorPtr;

		class TcpClient : public Noncopyable
		{
		public:
			TcpClient(Reactor* loop,
				const SocketAddress& serverAddr,
				const std::string& name);
			~TcpClient();

			void connect();
			void disconnect();
			void stop();

			TcpConnectionPtr   getConnection() const;
			Reactor*           getReactor() const;
			bool               retry() const;
			void               enableRetry();
			const std::string& name() const;

			void setConnectionCallback(const ConnectionCallback& cb);
			void setMessageCallback(const MessageCallback& cb);
			void setWriteCompleteCallback(const WriteCompleteCallback& cb);

		private:

			void newConnection(int sockfd);			
			void removeConnection(const TcpConnectionPtr& conn);
		private:

			Reactor*               mReactor;
			ConnectorPtr           mConnector; 
			const std::string      mName;
			ConnectionCallback     mConnectionCallback;
			MessageCallback        mMessageCallback;
			WriteCompleteCallback  mWriteCompleteCallback;

			bool                   mRetry;  
			bool                   mConnect; 
				
			int                    mNextConnId;
			
			TcpConnectionPtr       mConnection;
		};

	}
}

#endif

