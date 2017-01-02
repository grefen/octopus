#ifndef BUFFER_H
#define BUFFER_H

#include "Base.h"

namespace Octopus {

	class Buffer : public Copyable
	{
	public:

		static const size_t INIT_SIZE = 1024;

		explicit Buffer(size_t size = INIT_SIZE);

		void swap(Buffer& buffer);

		size_t availReadBytes() const;
		size_t availWriteBytes() const;

		const char* readableData() const;

		void recycle(size_t size);

		void append(const char* data, size_t size);

		void reallocMem(size_t size);

		void shrink(size_t reserve);

	private:
		std::vector<char> mBuffer;
		size_t            mWriteOffset;
		size_t            mReadOffset;
	};

}

#endif

