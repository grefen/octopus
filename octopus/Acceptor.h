#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Base.h"
#include "SocketAddress.h"
#include "Reactor.h"
#include "Socket.h"
#include "EventHandler.h"


namespace Octopus {

	namespace Core {

		typedef std::function<void(int sockfd, const SocketAddress&)> AcceptorCallback;

		class Acceptor : public Noncopyable
		{
		public:

			Acceptor(Reactor* reactor, const SocketAddress& addr);
			~Acceptor();

			void setConnectionCallback(AcceptorCallback cb);
			bool isListenning();
			void listen();

			void handleRead();
		private:

			Reactor*     mpReactor;
			Socket       macceptSocket;
			EventHandler macceptHandler;

			AcceptorCallback mconnectionCallback;

			bool         mlistenning;
			int          mreservedFd;
		};

	}

}

#endif

