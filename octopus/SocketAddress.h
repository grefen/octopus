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

		protected:

			bool tryParseIPV4(const std::string& addr, struct in_addr& ia);
			bool tryParseIPV6(const std::string& addr, struct in6_addr& ia, ULong& scope);

			void initIPV4(const struct in_addr& ia, UInt16 port);
			void initIPV6(const struct in6_addr& ia, ULong scope, UInt16 port);

			void init(const std::string& hostAddress, UInt16 portNumber);
			void initFromDNS(const std::string& hostAddress, UInt16 portNumber);
		private:

			union
			{
				struct sockaddr_in  mAddr;
				struct sockaddr_in6 mAddr6;
			}mAddress;

		};


	}
}

#endif

