#ifndef TYPES_H
#define TYPES_H

namespace Octopus {

	//unix/gcc

	typedef signed char              Int8;
	typedef unsigned char            UInt8;
	typedef signed  short            Int16;
	typedef unsigned short           UInt16;
	typedef signed  int              Int32;
	typedef unsigned int             UInt32;

	typedef signed long              IntPtr;
	typedef unsigned long            UIntPtr;

#if defined(__LP64__)
    #define OCTOPUS_PTR_IS_64_BIT 1
    #define OCTOPUS_LONG_IS_64_BIT 1
	typedef signed long              Int64;
	typedef unsigned long            UInt64;
#else
	typedef signed long long         Int64;
	typedef unsigned long long       UInt64;
#endif

    #define OCTOPUS_HAVE_INT64 1
}

#endif
