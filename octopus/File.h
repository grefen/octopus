#ifndef FILE_H
#define FILE_H

#include "Base.h"

namespace Octopus {

	class File
	{
	public:
		explicit File(const std::string& name);
		~File();

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

