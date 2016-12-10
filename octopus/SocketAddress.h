#ifndef SOCKETADDRESS_H
#define SOCKETADDRESS_H


#include "Base.h"


namespace Octopus {

	namespace Core {

		class SocketAddress : public Copyable
		{
		public:
			//Ĭ��ȫΪ0�ĵ�ַ��IPV4��ַ
			SocketAddress();
			//IP ��ַ�Ͷ˿ں�, IPV4��ַ��IPV6��ַ
			SocketAddress(const std::string& hostAddress, Octopus::UInt16 port);
			//IP ��ַ�Ͷ˿ںţ�IPV4��ַ��IPV6��ַ
			//�磺192.168.1.131:80�� [::ffff:192.168.1.131]:6666�� www.163.com:80
			SocketAddress(const std::string& hostAddressAndPort);
			//��ͨ�õ�ַ����
			explicit SocketAddress(const struct sockaddr* addr, Octopus::UInt32 length);


			//�������캯��
			SocketAddress(const SocketAddress& addr);

			~SocketAddress();


			SocketAddress& operator = (const SocketAddress& socketAddress);

			std::string     host() const;
			Octopus::UInt16 port() const;
			const struct sockaddr* addr() const;
			std::string     hostAndPort() const;

			Octopus::UInt32 length() const;

			int             family()const;

			enum
			{
				MAX_ADDRESS_LENGTH = sizeof(struct sockaddr_in6)				
			};

		private:

			union
			{
				struct sockaddr_in  mAddr;
				struct sockaddr_in6 mAdr6;
			}mAddress;

		};


	}
}

#endif

