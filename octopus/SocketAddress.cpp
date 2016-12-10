#include "SocketAddress.h"

namespace Octopus {
	namespace Core {

		SocketAddress::SocketAddress()
		{
			std::memset(&mAddress, 0, sizeof(mAddress));
			mAddress.mAddr.sin_family = AF_INET;

		}
		SocketAddress::SocketAddress(const std::string& hostAddress, Octopus::UInt16 port)
		{
			struct in_addr t4;
			struct in6_addr t6;
			ULong  scope = 0;

			if (tryParseIPV4(hostAddress, t4))
			{
				initIPV4(t4, port);
			}
			else if (tryParseIPV6(hostAddress, t6, scope))
			{
				initIPV6(t6, scope, port);
			}
			else
			{
				//get from DNS
				struct addrinfo* pAI;
				struct addrinfo hints;
				std::memset(&hints, 0, sizeof(hints));
				hints.ai_flags = AI_CANONNAME| AI_ADDRCONFIG;
				int rc = getaddrinfo(hostAddress.c_str(), NULL, &hints, &pAI);
				if (rc == 0)
				{
					std::vector<in_addr> ipv4;
					std::vector<in6_addr> ipv6;
					std::vector<ULong> scopId;
					for (struct addrinfo* ai = pAI; ai; ai = ai->ai_next)
					{
						if (ai->ai_canonname)
						{

						}
						if (ai->ai_addrlen && ai->ai_addr)
						{
							switch (ai->ai_addr->sa_family)
							{
							case AF_INET:
								ipv4.push_back(reinterpret_cast<struct sockaddr_in*>(ai->ai_addr)->sin_addr);
								break;

							case AF_INET6:
								ipv6.push_back(reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_addr);
								scopId.push_back(reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_scope_id));
								break;

							}
						}
					}

					freeaddrinfo(pAI);

					if (!ipv4.empty())
					{
						initIPV4(ipv4[0], port);
					}
					else if (!ipv6.empty())
					{
						initIPV6(ipv6[0], scopId[0], port);
					}
					else
					{
						//throw
					}

					
					
				}
				else
				{
					//aierror(rc, hostname);
					//throw
				}
			}
		}
		SocketAddress::SocketAddress(const std::string& hostAddressAndPort)
		{

		}
		SocketAddress::SocketAddress(const struct sockaddr* addr, Octopus::UInt32 length)
		{

		}

		SocketAddress::SocketAddress(const SocketAddress& addr)
		{

		}

		SocketAddress& SocketAddress::operator = (const SocketAddress& socketAddress)
		{

		}

		SocketAddress::~SocketAddress()
		{
		}

		std::string     SocketAddress::host() const
		{

		}
		Octopus::UInt16 SocketAddress::port() const
		{

		}
		const struct sockaddr* SocketAddress::addr() const
		{

		}
		std::string     SocketAddress::hostAndPort() const
		{

		}

		Octopus::UInt32 SocketAddress::length() const
		{

		}

		int             SocketAddress::family()const
		{

		}

		bool SocketAddress::tryParseIPV4(const std::string& addr, struct in_addr& ia)
		{
			struct in_addr t;
			if (inet_aton(addr.c_str(), &t))
			{
				memcpy(&ia, &t);
				return true;
			}

			return false;
		}
		bool SocketAddress::tryParseIPV6(const std::string& addr, struct in6_addr& ia, ULong& scId)
		{
			struct in6_addr ta;
			std::string::size_type pos = addr.find('%');
			if (std::string::npos != pos)
			{
				// fe80::210:5cff:fec8:aa1b%4
				std::string::size_type start = ('[' == addr[0]) ? 1 : 0;
				std::string unscopedAddr(addr, start, pos - start);
				std::string scope(addr, pos + 1, addr.size() - start - pos);
				Octopus::UInt32 scopedId = 0;
				if (!(scopeId = if_nametoindex(scope.c_str())))
				{
					return false;
				}
				if (inet_pton(AF_INET6, unscopedAddr.c_str(), &ta) == 1)
				{
					scId = scopeId;
					memcpy(&ia, &ta, sizeof(ta));
					return true;
				}

				return false;

			}
			else
			{
				if (inet_pton(AF_INET6, addr.c_str(), &ta))
				{
					memcpy(&ia, &ta, sizeof(ta));
					return true;
				}

				return false;
			}
		}

		void SocketAddress::initIPV4(const struct in_addr& ia, UInt16 port)
		{
			std::memset(&mAddress, 0, sizeof(mAddress));
			mAddress.mAddr.sin_family = AF_INET;
			std::memcpy(&mAddress.mAddr.sin_addr, &ia, sizeof(mAddress.mAddr.sin_addr));
			mAddress.mAddr.sin_port = port;
		}
		void SocketAddress::initIPV6(const struct in6_addr& ia, ULong scope, UInt16 port)
		{
			std::memset(&mAddress, 0, sizeof(mAddress));
			mAddress.mAddr6.sin6_family = AF_INET6;
			std::memcpy(&mAddress.mAddr6.sin6_addr, &ia, sizeof(mAddress.mAddr6.sin6_addr));
			mAddress.mAddr6.sin6_port = port;
			mAddress.mAddr6.sin6_scope_id = scope;
		}

	}
}

