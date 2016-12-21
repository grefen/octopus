#include "SocketAddress.h"
#include "String.h"

namespace Octopus {
	namespace Core {

		SocketAddress::SocketAddress()
		{
			std::memset(&mAddress, 0, sizeof(mAddress));
			mAddress.mAddr.sin_family = AF_INET;

		}
		SocketAddress::SocketAddress(const std::string& hostAddress, Octopus::UInt16 port)
		{
			init(hostAddress, port);
		}
		SocketAddress::SocketAddress(const std::string& hostAddressAndPort)
		{

			std::string host;
			std::string port;
			std::string::const_iterator it = hostAddressAndPort.begin();
			std::string::const_iterator end = hostAddressAndPort.end();
			if (*it == '[')
			{
				++it;
				while (it != end && *it != ']') host += *it++;
				if (it == end) throw InvalidArgumentException("Malformed IPv6 address");
				++it;
			}
			else
			{
				while (it != end && *it != ':') host += *it++;
			}
			if (it != end && *it == ':')
			{
				++it;
				while (it != end) port += *it++;
			}
			else throw InvalidArgumentException("Missing port number");
			init(host, (UInt16)std::stoi(port));
		}
		SocketAddress::SocketAddress(const struct sockaddr* addr, Octopus::UInt32 length)
		{
			if (length == sizeof(struct sockaddr_in))				
			   std::memcpy(&mAddress.mAddr, addr, length);
			else if (length == sizeof(struct sockaddr_in6))
				std::memcpy(&mAddress.mAddr6, addr, length);				
			else throw InvalidArgumentException("Invalid address length passed to SocketAddress()");
		}

		SocketAddress::SocketAddress(const SocketAddress& addr)
		{
			std::memcpy(&mAddress.mAddr6, &addr.mAddress.mAddr6, MAX_ADDRESS_LENGTH);
		}

		SocketAddress& SocketAddress::operator = (const SocketAddress& socketAddress)
		{
			if (&socketAddress != this)
			{				
				std::memcpy(&mAddress.mAddr6, &socketAddress.mAddress.mAddr6, MAX_ADDRESS_LENGTH);
			}
			return *this;
		}

		SocketAddress::~SocketAddress()
		{
		}

		std::string     SocketAddress::host() const
		{
			char buf[64] = "";
			if (family() == AF_INET)
			{
				const struct sockaddr_in* addr4 = &mAddress.mAddr;
				::inet_ntop(AF_INET, &addr4->sin_addr, buf, 63);

				return buf;
			}
			else
			{
				const struct sockaddr_in6* addr6 = &mAddress.mAddr6;
				::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, 63);
				return buf;
			}

			return buf;
		}
		Octopus::UInt16 SocketAddress::port() const
		{
			return ntohs(mAddress.mAddr.sin_port);
		}
		const struct sockaddr* SocketAddress::addr() const
		{
			return reinterpret_cast<const struct sockaddr*>(&mAddress.mAddr);
		}
		std::string     SocketAddress::hostAndPort() const
		{
			char buf[64] = "";

			std::string hostAddr = host();

			size_t end = hostAddr.length();
			const struct sockaddr_in* addr4 = &mAddress.mAddr;
			UInt16 port = ntohs(addr4->sin_port);
			
			snprintf(buf + end, sizeof(buf) - end, ":%u", port);

			return buf;
		}

		Octopus::UInt32 SocketAddress::length() const
		{
			if (family() == AF_INET) return sizeof(sockaddr_in);

			return sizeof(sockaddr_in6);
		}

		int             SocketAddress::family()const
		{
			return mAddress.mAddr.sin_family;
		}

		bool SocketAddress::tryParseIPV4(const std::string& addr, struct in_addr& ia)
		{
			struct in_addr t;
			if (inet_aton(addr.c_str(), &t))
			{
				memcpy(&ia, &t, sizeof(in_addr));
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
				Octopus::UInt32 scopeId = 0;
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
			mAddress.mAddr.sin_port = htons(port);
		}
		void SocketAddress::initIPV6(const struct in6_addr& ia, ULong scope, UInt16 port)
		{
			std::memset(&mAddress, 0, sizeof(mAddress));
			mAddress.mAddr6.sin6_family = AF_INET6;
			std::memcpy(&mAddress.mAddr6.sin6_addr, &ia, sizeof(mAddress.mAddr6.sin6_addr));
			mAddress.mAddr6.sin6_port = htons(port);
			mAddress.mAddr6.sin6_scope_id = (UInt32)scope;
		}

		void SocketAddress::init(const std::string& hostAddress, UInt16 portNumber)
		{
			struct in_addr t4;
			struct in6_addr t6;
			ULong  scope = 0;

			if (tryParseIPV4(hostAddress, t4))
			{
				initIPV4(t4, portNumber);
			}
			else if (tryParseIPV6(hostAddress, t6, scope))
			{
				initIPV6(t6, scope, portNumber);
			}
			else
			{
				//get from DNS
				initFromDNS(hostAddress, portNumber);

			}
		}

		void SocketAddress::initFromDNS(const std::string& hostAddress, UInt16 portNumber)
		{
			struct addrinfo* pAI;
			struct addrinfo hints;
			std::memset(&hints, 0, sizeof(hints));
			hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
			Int32 rc = getaddrinfo(hostAddress.c_str(), NULL, &hints, &pAI);
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
							scopId.push_back(reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_scope_id);
							break;

						}
					}
				}

				freeaddrinfo(pAI);

				if (!ipv4.empty())
				{
					initIPV4(ipv4[0], portNumber);
				}
				else if (!ipv6.empty())
				{
					initIPV6(ipv6[0], scopId[0], portNumber);
				}
				else
				{
					//throw
					throw HostNotFoundException("No address found for host", hostAddress);
				}

			}
			else
			{

				switch (rc)
				{
				case EAI_AGAIN:
					throw DNSException("Temporary DNS error while resolving", hostAddress);
				case EAI_FAIL:
					throw DNSException("Non recoverable DNS error while resolving", hostAddress);
				case EAI_NONAME:
					throw HostNotFoundException(hostAddress);
				default:
					throw DNSException("EAI", toString(rc));
				}
			}
		}

	}
}

