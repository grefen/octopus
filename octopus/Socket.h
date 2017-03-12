#ifndef SOCKET_H
#define SOCKET_H

#include "Base.h"



namespace Octopus {

	namespace Core {

		class SocketAddress;

		class Socket
		{
		public:
			explicit Socket(int fd);
			~Socket();

			int fd() const { return mfd; };

			void bind(const SocketAddress& address);

			void listen();

			int  accept(SocketAddress& peeraddr);

			void shutdownWrite();

			void setTcpNoDelay(bool on);

			void setReuseAddr(bool on);

			void setReusePort(bool on);

			void setKeepAlive(bool on);

			static const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
			static struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);

		private:
			const int mfd;
		};

	}
}

#endif

