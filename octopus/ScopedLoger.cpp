#include "ScopedLoger.h"
#include "TimeZone.h"
#include "TypeFormat.h"


namespace Octopus {

	const char* LogLevelName[ScopedLoger::NUM_LOG_LEVELS] =
	{
		"TRACE ",
		"DEBUG ",
		"INFO  ",
		"WARN  ",
		"ERROR ",
		"FATAL ",
	};

	void defaultOutput(const char* msg, size_t len)
	{
		fwrite(msg, 1, len, stdout);
	}

	void defaultFlush()
	{
		fflush(stdout);
	}

	ScopedLoger::Level g_globalLevel = ScopedLoger::TRACE;

	ScopedLoger::OutputFunc g_output = defaultOutput;
	ScopedLoger::FlushFunc g_flush = defaultFlush;
	TimeZone g_logTimeZone;

	ScopedLoger::Level ScopedLoger::getLogLevel()
	{
		return g_globalLevel;
	}
	void ScopedLoger::setLogLevel(ScopedLoger::Level l)
	{
		g_globalLevel = l;
	}

	void ScopedLoger::setOutput(OutputFunc out)
	{
		g_output = out;
	}
	void ScopedLoger::setFlush(FlushFunc flush)
	{
		g_flush = flush;
	}

	void ScopedLoger::setTimeZone(const TimeZone& tz)
	{
		g_logTimeZone = tz;
	}

	StreamBuffer& ScopedLoger::stream()
	{
		return mStream;
	}

	ScopedLoger::ScopedLoger(const char* filename, int line)
		:mTime(Timestamp::now()),
		mStream(),
		mLevel(TRACE),
		mLine(line),
		mFileName(filename)
	{
		init();
	}
	ScopedLoger::ScopedLoger(const char* filename, int line, int level)
		:mTime(Timestamp::now()),
		mStream(),
		mLevel((ScopedLoger::Level)level),
		mLine(line),
		mFileName(filename)
	{
		init();
	}
	ScopedLoger::ScopedLoger(const char* filename, int line, int level, const char* fun)
		:mTime(Timestamp::now()),
		mStream(),
		mLevel((ScopedLoger::Level)level),
		mLine(line),
		mFileName(filename)
	{
		init();

		mStream << fun << ' ';
	}
	ScopedLoger::ScopedLoger(const char* filename, int line, bool toAbort)
		:mTime(Timestamp::now()),
		mStream(),
		mLevel(toAbort ? FATAL : ERROR),
		mLine(line),
		mFileName(filename)
	{
		init();
	}


	ScopedLoger::~ScopedLoger()
	{
		mStream << " - " << mFileName << ':' << mLine << '\n';
	
		g_output(stream().data(), stream().length());
		if (mLevel == FATAL)
		{
			g_flush();
			abort();
		}
	}

	void ScopedLoger::init()
	{
		int64_t microSecondsSinceEpoch = mTime.epochMicroseconds();
		time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::TIME_RESOLUTION);
		int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::TIME_RESOLUTION);

		struct tm tm_time;
		gmtime_r(&seconds, &tm_time);

		char t_time[32] = { 0 };
		snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

		char t_micro[32] = { 0 };
		snprintf(t_micro, sizeof(t_micro), ".%06dZ ", microseconds);

		mStream.append(t_time, 17);
		mStream.append(t_micro, 9);
		mStream.append(LogLevelName[mLevel], 6);
	}

}
