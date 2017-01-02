#include "Buffer.h"

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

}
