#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "Base.h"
#include "TypeFormat.h"

namespace Octopus {


	class StreamBuffer : public Noncopyable
	{
	public:
		explicit StreamBuffer(size_t size = DefaultBufferSize);
		~StreamBuffer();

		const static int DefaultBufferSize = 4000;

		StreamBuffer& operator<<(bool v);
		StreamBuffer& operator<<(char v);
		//StreamBuffer& operator<<(unsigned char v);
		StreamBuffer& operator<<(short v);
		StreamBuffer& operator<<(unsigned short v);
		StreamBuffer& operator<<(int v);
		StreamBuffer& operator<<(unsigned int v);
		StreamBuffer& operator<<(long v);
		StreamBuffer& operator<<(unsigned long v);
		StreamBuffer& operator<<(unsigned long long v);
		//StreamBuffer& operator<<(float v);
		StreamBuffer& operator<<(double v);

		StreamBuffer& operator<<(const char* v);
		StreamBuffer& operator<<(const unsigned char* v);

		StreamBuffer& operator<<(const std::string& v);

		StreamBuffer& operator<<(const void*);

		template<typename T>
		void appendInteger(T v);
		
		void        append(const char* buf, size_t size);
		const char* data() const;
		size_t      length() const;
		void        reset() ;

	protected:

		size_t    avail();

	private:

		std::vector<char> mBuffer;
		size_t            mCur;
	};
	inline StreamBuffer& operator<<(StreamBuffer& s, const TypeFormat& fmt)
	{
		s.append(fmt.data(), fmt.length());

		return s;
	}
}



#endif

