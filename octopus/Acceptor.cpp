#include "Acceptor.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h> 
#include <strings.h>  
#include <sys/socket.h>
#include <unistd.h>

#include "SocketAddress.h"
#include "Log.h"


namespace Octopus {

	int createNonblockingSocket(int family)
	{
		int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
		if (sockfd < 0)
		{
			LOG_SYSFATAL << "sockets::createNonblockingOrDie";
		}

		return sockfd;
	}

	namespace Core {

		Acceptor::Acceptor(Reactor* reactor, const SocketAddress& addr)
			:mpReactor(reactor)
			, macceptSocket(createNonblockingSocket(addr.family()))
			, macceptHandler(reactor, macceptSocket.fd())
			, mlistenning(false)
			, mreservedFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
		{
			macceptSocket.setReuseAddr(true);
			macceptSocket.setReusePort(true);
			macceptSocket.bind(addr);
			macceptHandler.setReadCallback(std::bind(&Acceptor::handleRead, this));
		}


		Acceptor::~Acceptor()
		{
			macceptHandler.disableAll();
			macceptHandler.remove();
			::close(mreservedFd);
		}

		void Acceptor::setConnectionCallback(ConnectionCallback cb)
		{
			mconnectionCallback = cb;
		}
		bool Acceptor::isListenning()
		{
			return mlistenning;
		}
		void Acceptor::listen()
		{
			mlistenning = true;
			macceptSocket.listen();
			macceptHandler.enableRead(true);
		}

		void Acceptor::handleRead()
		{
			
			SocketAddress peerAddr;
			
			int connfd = macceptSocket.accept(peerAddr);
			if (connfd >= 0)
			{

				if (mconnectionCallback)
				{
					mconnectionCallback(connfd, peerAddr);
				}
				else
				{
					if (::close(connfd) < 0)
					{
						LOG_SYSERR << "sockets::close";
					}
				}
			}
			else
			{
				LOG_SYSERR << "in Acceptor::handleRead";

				//������ľ����ر�Ԥ����mreservedFd���������Կճ�һ�����
				//Ȼ��accept���ٹرգ��ٴ���Ԥ��mreservedFd
				//�������Խ������ľ�����
				if (errno == EMFILE)
				{
					::close(mreservedFd);
					mreservedFd = ::accept(macceptSocket.fd(), NULL, NULL);
					::close(mreservedFd);
					mreservedFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
				}
			}
		}
	}
}
