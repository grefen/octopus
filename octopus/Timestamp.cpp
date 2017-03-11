
#include "Timestamp.h"
#include <sys/time.h>

namespace Octopus {

	Timestamp::Timestamp():mValue(0)
	{}
	Timestamp::Timestamp(Int64 val): mValue(val)
	{}
	Timestamp::Timestamp(const Timestamp& stamp)
	{
		mValue = stamp.mValue;
	}
	Timestamp::~Timestamp()
	{}
	Timestamp& Timestamp::operator=(const Timestamp& stamp)
	{
		mValue = stamp.mValue;
		return *this;
	}
	Timestamp& Timestamp::operator=(Int64 stamp)
	{
		mValue = stamp;
		return *this;
	}

	void Timestamp::swap(Timestamp& stamp)
	{
		std::swap(mValue, stamp.mValue);
	}

	bool Timestamp::operator == (const Timestamp& ts) const
	{
		return mValue == ts.mValue;
	}
	bool Timestamp::operator != (const Timestamp& ts) const
	{
		return mValue != ts.mValue;
	}
	bool Timestamp::operator >  (const Timestamp& ts) const
	{
		return mValue > ts.mValue;
	}
	bool Timestamp::operator >= (const Timestamp& ts) const
	{
		return mValue >= ts.mValue;
	}
	bool Timestamp::operator <  (const Timestamp& ts) const
	{
		return mValue < ts.mValue;
	}
	bool Timestamp::operator <= (const Timestamp& ts) const
	{
		return mValue <= ts.mValue;
	}

	Timestamp  Timestamp::operator +  (Int64 d) const
	{
		return Timestamp(mValue + d);
	}
	Timestamp  Timestamp::operator -  (Int64 d) const
	{
		return Timestamp(mValue - d);
	}
	Int64   Timestamp::operator -  (const Timestamp& ts) const
	{
		return mValue - ts.mValue;
	}
	Timestamp& Timestamp::operator += (Int64 d)
	{
		mValue += d;
		return *this;
	}
	Timestamp& Timestamp::operator -= (Int64 d)
	{
		mValue -= d;
		return *this;
	}

	Int64   Timestamp::epochMicroseconds() const
	{
		return mValue;
	}

	std::time_t Timestamp::epochTime() const
	{
		return std::time_t(mValue / TIME_RESOLUTION);
	}

	Int64 Timestamp::utcTime() const
	{
		return mValue * 10 + (Int64(0x01b21dd2) << 32) + 0x13814000;
	}

	std::string Timestamp::toString() const
	{
		char buf[32] = { 0 };
		Int64 seconds = mValue / TIME_RESOLUTION;
		Int64 microseconds = mValue % TIME_RESOLUTION;
		snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
		return buf;
	}

	std::string  Timestamp::toData() const
	{
		char buf[32] = { 0 };
		time_t seconds = static_cast<time_t>(mValue / TIME_RESOLUTION);
		struct tm tm_time;
		gmtime_r(&seconds, &tm_time);

		int microseconds = static_cast<int>(mValue % TIME_RESOLUTION);
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			microseconds);

		return buf;
	}

	Timestamp Timestamp::now()
	{
		struct timeval tv;
		if (gettimeofday(&tv, NULL))
			throw SystemException("cannot get time of day");
		UInt64 value = Int64(tv.tv_sec)*TIME_RESOLUTION + tv.tv_usec;

		return Timestamp(value);
	}

	Timestamp Timestamp::fromEpochTime(std::time_t t)
	{
		return Timestamp(Int64(t)*TIME_RESOLUTION);
	}

	Timestamp Timestamp::fromUtcTime(Int64 val)
	{
		val -= (Int64(0x01b21dd2) << 32) + 0x13814000;
		val /= 10;
		return Timestamp(val);
	}

}

