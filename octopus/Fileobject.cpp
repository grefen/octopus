#include "Fileobject.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

namespace Octopus {

	Fileobject::Fileobject(const std::string& name)
	{
		mfp = fopen(name.c_str(), "ae");
		setbuffer(mfp, mBuffer, sizeof(mBuffer));
		mWriteSize = 0;
	}

	Fileobject::~Fileobject()
	{
		fclose(mfp);
	}

	void Fileobject::write(const char* line, size_t size)
	{
		size_t len    = 0;
		size_t offset = 0;
		size_t remain = size;
		while (remain > 0)
		{
			len = fwrite_unlocked(line + offset, 1, remain, mfp);
			if (len == 0)
			{
				int err = ferror(mfp);
				if (err)
				{
					char buf[512];
					fprintf(stderr, "File::write failed %s\n", strerror_r(err, buf, sizeof(buf)));
				}
				break;
			}
			offset += len;
			remain = remain - len;
		}

		mWriteSize += size;
	}

	void Fileobject::flush()
	{
		fflush(mfp);
	}

}
