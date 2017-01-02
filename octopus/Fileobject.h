#ifndef FILE_H
#define FILE_H

#include "Base.h"
#include "Noncopyable.h"

namespace Octopus {

	class Fileobject : public Noncopyable
	{
	public:
		explicit Fileobject(const std::string& name);
		~Fileobject();

		static const UInt32 BUFFER_SIZE = 64 * 1024;//64KB

		void write(const char* line, size_t size);
		void flush();

	private:

		FILE*  mfp;
		char   mBuffer[BUFFER_SIZE];
		size_t mWriteSize;
	};

}

#endif

