#include "StreamBuffer.h"

namespace Octopus {

	const char digits[] = "9876543210123456789";
	const char* zero = digits + 9;

	const char digitsHex[] = "0123456789ABCDEF";

	static const int MaxNumericSize = 32;

	// Efficient Integer to String Conversions, by Matthew Wilson.
	template<typename T>
	size_t convert(char buf[], T value)
	{
		T i = value;
		char* p = buf;

		do
		{
			int lsd = static_cast<int>(i % 10);
			i /= 10;
			*p++ = zero[lsd];
		} while (i != 0);

		if (value < 0)
		{
			*p++ = '-';
		}
		*p = '\0';
		std::reverse(buf, p);

		return p - buf;
	}

	size_t convertHex(char buf[], uintptr_t value)
	{
		uintptr_t i = value;
		char* p = buf;

		do
		{
			int lsd = static_cast<int>(i % 16);
			i /= 16;
			*p++ = digitsHex[lsd];
		} while (i != 0);

		*p = '\0';
		std::reverse(buf, p);

		return p - buf;
	}

	template<typename T>
	void StreamBuffer::appendInteger(T v)
	{
		if (avail() >= MaxNumericSize)
		{
			size_t len = convert(&mBuffer[mCur], v);
			mCur += len;
		}
	}

	StreamBuffer::StreamBuffer(size_t size)
	{
		mBuffer.resize(size);
		mCur = 0;
	}
	StreamBuffer::~StreamBuffer()
	{

	}

	StreamBuffer& StreamBuffer::operator<<(bool v)
	{
		append(v ? "1" : "0", 1);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(char v)
	{
		append(&v , 1);
		return *this;
	}
	//StreamBuffer& StreamBuffer::operator<<(unsigned char v)
	//{		
	//	append(&v, 1);
	//	return *this;
	//}
	StreamBuffer& StreamBuffer::operator<<(short v)
	{
		*this << static_cast<int>(v);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(unsigned short v)
	{
		*this << static_cast<unsigned int>(v);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(int v)
	{
		appendInteger(v);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(unsigned int v)
	{
		appendInteger(v);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(long v)
	{
		appendInteger(v);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(unsigned long v)
	{
		appendInteger(v);
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(unsigned long long v)
	{
		appendInteger(v);
		return *this;
	}
	//StreamBuffer& StreamBuffer::operator<<(float v)
	//{

	//}
	StreamBuffer& StreamBuffer::operator<<(double v)
	{
		if (avail() >= MaxNumericSize)
		{
			int len = snprintf(&mBuffer[mCur], MaxNumericSize, "%.12g", v);
			mCur += len;
		}
		return *this;
	}

	StreamBuffer& StreamBuffer::operator<<(const char* v)
	{
		if (v)
		{
			append(v, strlen(v));
		}
		else
		{
			append("null", 6);
		}
		return *this;
	}
	StreamBuffer& StreamBuffer::operator<<(const unsigned char* v)
	{
		return operator<<(reinterpret_cast<const char*>(v));
	}

	StreamBuffer& StreamBuffer::operator<<(const std::string& v)
	{
		append(v.c_str(), v.size());
		return *this;
	}

	StreamBuffer& StreamBuffer::operator<<(const void* p)
	{
		uintptr_t v = reinterpret_cast<uintptr_t>(p);
		if (avail() >= MaxNumericSize)
		{
			char* buf = &mBuffer[mCur];
			buf[0] = '0';
			buf[1] = 'x';
			size_t len = convertHex(buf + 2, v);
			
			mCur += len + 2;
		}
		return *this;
	}

	void        StreamBuffer::append(const char* buf, size_t size)
	{
		if (static_cast<size_t>(mBuffer.size() - mCur) > size)
		{
			memcpy(&mBuffer[mCur], buf, size);
			mCur += size;
		}
	}
	const char* StreamBuffer::data() const
	{
		return &mBuffer[0];
	}
	size_t      StreamBuffer::length() const
	{
		return mBuffer.size();
	}
	void        StreamBuffer::reset() 
	{		
		memset(&mBuffer[0], 0, mBuffer.size());
		mCur = 0;
	}

	size_t    StreamBuffer::avail()
	{
		return mBuffer.size() - mCur;
	}

}
