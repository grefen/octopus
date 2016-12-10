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

		}

		SocketAddress::~SocketAddress()
		{
		}

	}
}

