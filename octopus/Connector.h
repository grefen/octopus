#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "Base.h"
#include "SocketAddress.h"
#include "Reactor.h"
#include "Socket.h"
#include "EventHandler.h"
#include "Buffer.h"

namespace Octopus {

	namespace Core {

		typedef std::function<void(int sockfd)> NewConnectionCallback;

		class Connector : public Noncopyable, public std::enable_shared_from_this<Connector>
		{
		public:			

			Connector(Reactor* pReactor, const SocketAddress& address);
			~Connector();

			void setConnectionCallback(const NewConnectionCallback& cb);

			const SocketAddress& getServerAddress();

			void start();  // can be called in any thread
			void restart();  // must be called in loop thread
			void stop();  // can be called in any thread
		protected:

			enum ConnectState{
				eDisconnected = 0,
				eConnecting,
				eConnected,
			};
			
			//
			void startInReactor();
			void stopInReactor();

			//����������
			void connect();
			void retry(int sockfd);
			
			void handleWrite();
			void handleError();			
			
			//��EventHandler����
			void bindEventHandler(int sockfd);
			int  unbindEventHandler();
			void resetEventHandler();

		private:

			Reactor*      mpReactor;
			SocketAddress mAddress;

			NewConnectionCallback mConnectionCallback;

			std::unique_ptr<EventHandler> mEventHandler;

			bool         mConnect;//���ӹ����н�������

			ConnectState mConnectState;		

			int          mretryDelayMs;
		};

	}
}

#endif

