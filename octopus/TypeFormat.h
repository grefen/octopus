#ifndef TYPEFORMAT_H
#define TYPEFORMAT_H

#include "Base.h"

namespace Octopus {

	
	class TypeFormat
	{
	public:
		template<typename T>
		TypeFormat(const char* fmt, T val)
		{
			mSize = snprintf(mBuf, sizeof(mBuf), fmt, val);
			
		}

		const char* data() const
		{
			return mBuf;
		}
		size_t   length() const
		{
			return mSize;
		}

	private:
		char   mBuf[32];//
		size_t mSize;
	};

}

#endif

