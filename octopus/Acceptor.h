#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Base.h"
#include "SocketAddress.h"
#include "Reactor.h"
#include "Socket.h"
#include "EventHandler.h"


namespace Octopus {

	namespace Core {

		typedef std::function<void(int sockfd, const SocketAddress&)> ConnectionCallback;

		class Acceptor : public Noncopyable
		{
		public:

			Acceptor(Reactor* reactor, const SocketAddress& addr);
			~Acceptor();

			void setConnectionCallback(ConnectionCallback cb);
			bool isListenning();
			void listen();

			void handleRead();
		private:

			Reactor*     mpReactor;
			Socket       macceptSocket;
			EventHandler macceptHandler;

			ConnectionCallback mconnectionCallback;

			bool         mlistenning;
			int          mreservedFd;
		};

	}

}

#endif

