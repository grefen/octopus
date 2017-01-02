#ifndef SCOPEDLOGER_H
#define SCOPEDLOGER_H

#include "Base.h"
#include "TypeFormat.h"
#include "Timestamp.h"
#include "StreamBuffer.h"
#include "TimeZone.h"


namespace Octopus {

	class ScopedLoger
	{
	public:
		enum Level
		{
			TRACE,
			DEBUG,
			INFO,
			WARN,
			ERROR,
			FATAL,
			NUM_LOG_LEVELS,
		};

		ScopedLoger(const char* filename, int line);
		ScopedLoger(const char* filename, int line, int level);
		ScopedLoger(const char* filename, int line, int level, const char* fun);
		ScopedLoger(const char* filename, int line, bool toAbort);

		~ScopedLoger();

		StreamBuffer& stream();

		static Level getLogLevel();
		static void  setLogLevel(Level l);

		typedef void(*OutputFunc)(const char* msg, size_t len);
		typedef void(*FlushFunc)();
		static void setOutput(OutputFunc);
		static void setFlush(FlushFunc);

		static void setTimeZone(const TimeZone& tz);
	protected:

		void init();
	private:

		Timestamp    mTime;
		StreamBuffer mStream;
		Level        mLevel;
		int          mLine;
		const char*        mFileName;
	};

}

#endif

