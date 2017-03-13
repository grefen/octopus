#include "Buffer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h> 
#include <strings.h>  
#include <sys/socket.h>
#include <unistd.h>

namespace Octopus {

	Buffer::Buffer(size_t size):
		mBuffer(size),
		mWriteOffset(0),
		mReadOffset(0)
	{

	}


	void Buffer::swap(Buffer& buffer)
	{
		mBuffer.swap(buffer.mBuffer);
		std::swap(mWriteOffset, buffer.mWriteOffset);
		std::swap(mReadOffset, buffer.mReadOffset);
	}

	size_t Buffer::availReadBytes() const
	{
		return mWriteOffset - mReadOffset;
	}

	size_t Buffer::availWriteBytes() const
	{
		return mBuffer.size() - mWriteOffset;
	}

	const char* Buffer::readableData() const
	{
		return &mBuffer[0] + mReadOffset;
	}

	void Buffer::recycle(size_t size)
	{
		if (size < availReadBytes())
		{
			mReadOffset += size;
		}
		else
		{
			mReadOffset = 0;
			mWriteOffset = 0;
		}
	}

	void Buffer::append(const char* data, size_t size)
	{
		if (availWriteBytes() < size)
		{
			reallocMem(size);
		}

		std::copy(data, data + size, &mBuffer[0] + size);
		mWriteOffset += size;
	}

	void Buffer::reallocMem(size_t size)
	{
		if (availWriteBytes() + mReadOffset < size)
		{
			//如果前部空间加可写空间大小不足，则重新分配
			mBuffer.resize(mWriteOffset + size);
		}
		else
		{
			//把占用空间前移
			size_t readable = mReadOffset;
			std::copy(mBuffer.begin() + mReadOffset, mBuffer.begin() + mWriteOffset, mBuffer.begin());
			mReadOffset = 0;
			mWriteOffset = mReadOffset + readable;
		}
	}

	void Buffer::shrink(size_t reserve)
	{
		Buffer temp;
		temp.reallocMem(availReadBytes() + reserve);
		temp.append(readableData(), availReadBytes());

		swap(temp);
	}

	ssize_t Buffer::readFd(int fd, int* savedErrno)
	{
		
		char extrabuf[65536];
		struct iovec vec[2];
		const size_t writable = availWriteBytes();
		vec[0].iov_base = &mBuffer[0] + mWriteOffset;
		vec[0].iov_len = writable;
		vec[1].iov_base = extrabuf;
		vec[1].iov_len = sizeof extrabuf;

		//有两部分缓存，一个是bufffer中的，另一个是extrabuf中的；
		//当buffer中有足够内存，不需要extrabuf
		//当需要extrabuf时，把extrabuf中的append到当前buffer中，append时buffer自动扩展内存

		const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
		const ssize_t n = ::readv(fd, vec, iovcnt); 
		if (n < 0)
		{
			*savedErrno = errno;
		}
		else if (static_cast<size_t>(n) <= writable)
		{
			mWriteOffset += n;
		}
		else
		{
			mWriteOffset = mBuffer.size();
			append(extrabuf, n - writable);
		}

		return n;
	}

	const char* Buffer::peek() const
	{
		return &mBuffer[0] + mReadOffset;
	}

	void Buffer::retrieveAll()
	{
		mReadOffset = 0;
		mWriteOffset = 0;
	}

}
