#ifndef BASE_H
#define BASE_H

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>


#include <net/if.h>


#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include<sys/time.h>
#include <inttypes.h>  


#include <stdexcept>
#include <typeinfo>
#include <cctype>
#include <cstring>
#include <string>
#include <memory>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <map>
#include <set>
#include <atomic> 
#include <cassert>


#include "Types.h"
#include "Copyable.h"
#include "Noncopyable.h"
#include "Exception.h"

#endif
