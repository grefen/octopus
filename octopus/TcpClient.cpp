#include "TcpClient.h"

#include "Log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <unistd.h>


namespace Octopus {

	namespace Core {

		static void removeConnectionInReactor(Reactor* loop, const TcpConnectionPtr& conn)
		{
			loop->queuePending(std::bind(&TcpConnection::connectionDestroyed, conn));
		}

		static void removeConnector(const ConnectorPtr& connector)
		{
			//connector->
		}

		TcpClient::TcpClient(Reactor* loop,
			const SocketAddress& serverAddr,
			const std::string& name)
			: mReactor(loop),
			mConnector(new Connector(loop, serverAddr)),
			mName(name),
			mConnectionCallback(defaultConnectionCallback),
			mMessageCallback(defaultMessageCallback),
			mRetry(false),
			mConnect(true),
			mNextConnId(1)
		{
			mConnector->setConnectionCallback(
				std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
			
			LOG_INFO << "TcpClient::TcpClient[" << mName
				<< "] - connector " ;
		}

		TcpClient::~TcpClient()
		{
			LOG_INFO << "TcpClient::~TcpClient[" << mName
				<< "] - connector " ;

			TcpConnectionPtr conn;
			bool unique = false;
			{
				unique = mConnection.unique();
				conn = mConnection;
			}
			if (conn)
			{

				CloseCallback cb = std::bind(removeConnectionInReactor, mReactor, std::placeholders::_1);
				mReactor->runInReactor(
					std::bind(&TcpConnection::setCloseCallback, conn, cb));
				if (unique)
				{
					conn->forceClose();
				}
			}
			else
			{
				mConnector->stop();
				
				mReactor->setDelayTimer(1, std::bind(&removeConnector, mConnector));
			}
		}

		void TcpClient::connect()
		{
			LOG_INFO << "TcpClient::connect[" << mName << "] - connecting to "
				<< mConnector->getServerAddress().hostAndPort();

			mConnect = true;
			mConnector->start();
		}

		void TcpClient::disconnect()
		{
			mConnect = false;

			if (mConnection)
			{
				mConnection->shutdown();
			}
		}

		void TcpClient::stop()
		{
			mConnect = false;
			mConnector->stop();
		}

		void TcpClient::newConnection(int sockfd)
		{
			struct sockaddr_in6 peeraddr;
			bzero(&peeraddr, sizeof peeraddr);
			socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
			if (::getpeername(sockfd, static_cast<struct sockaddr*>((void*)&peeraddr), &addrlen) < 0)
			{
				LOG_SYSERR << "sockets::getPeerAddr";
			}			

			SocketAddress peerAddr;
			peerAddr.setSockAddrInet6(peeraddr);

			char buf[32];
			snprintf(buf, sizeof buf, ":%s#%d", peerAddr.hostAndPort().c_str(), mNextConnId);
			++mNextConnId;
			std::string connName = mName + buf;

			struct sockaddr_in6 localaddr;
			bzero(&localaddr, sizeof localaddr);
			addrlen = static_cast<socklen_t>(sizeof localaddr);
			if (::getsockname(sockfd, static_cast<struct sockaddr*>((void*)&localaddr), &addrlen) < 0)
			{
				LOG_SYSERR << "sockets::getLocalAddr";
			}			

			SocketAddress localAddr;
			peerAddr.setSockAddrInet6(localaddr);

			TcpConnectionPtr conn(new TcpConnection(mReactor,
				connName,
				sockfd,
				localAddr,
				peerAddr));

			conn->setConnectionCallback(mConnectionCallback);
			conn->setMessageCallback(mMessageCallback);
			conn->setWriteCompleteCallback(mWriteCompleteCallback);
			conn->setCloseCallback(
				std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));

			mConnection = conn;
			
			conn->connectionEstablished();
		}

		void TcpClient::removeConnection(const TcpConnectionPtr& conn)
		{

			mConnection.reset();
			

			mReactor->queuePending(std::bind(&TcpConnection::connectionDestroyed, conn));
			if (mRetry && mConnect)
			{
				LOG_INFO << "TcpClient::connect[" << mName << "] - Reconnecting to "
					<< mConnector->getServerAddress().hostAndPort();
				mConnector->restart();
			}
		}

		TcpConnectionPtr TcpClient::getConnection() const
		{
			return mConnection;
		}

		Reactor* TcpClient::getReactor() const
		{ 
			return mReactor;
		}
		bool TcpClient::retry() const 
		{ 
			return mRetry;
		}
		void TcpClient::enableRetry()
		{ 
			mRetry = true;
		}

		const std::string& TcpClient::name() const
		{
			return mName;
		}

		void TcpClient::setConnectionCallback(const ConnectionCallback& cb)
		{
			mConnectionCallback = cb;
		}

		void TcpClient::setMessageCallback(const MessageCallback& cb)
		{
			mMessageCallback = cb;
		}

		void TcpClient::setWriteCompleteCallback(const WriteCompleteCallback& cb)
		{
			mWriteCompleteCallback = cb;
		}

	}
}