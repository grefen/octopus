#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "Base.h"
#include "Copyable.h"

namespace Octopus {

	class Timestamp : public Copyable
	{
	public:
		Timestamp();
		Timestamp(Int64 val);

		Timestamp(const Timestamp& stamp);

		~Timestamp();

		Timestamp& operator=(const Timestamp& stamp);
		Timestamp& operator=(Int64 stamp);

		void swap(Timestamp& stamp);

		bool operator == (const Timestamp& ts) const;
		bool operator != (const Timestamp& ts) const;
		bool operator >  (const Timestamp& ts) const;
		bool operator >= (const Timestamp& ts) const;
		bool operator <  (const Timestamp& ts) const;
		bool operator <= (const Timestamp& ts) const;

		Timestamp  operator +  (Int64 d) const;
		Timestamp  operator -  (Int64 d) const;
		Int64   operator -  (const Timestamp& ts) const;
		Timestamp& operator += (Int64 d);
		Timestamp& operator -= (Int64 d);

		static const Int64 TIME_RESOLUTION = 1000 * 1000;

		Int64  epochMicroseconds() const;
		std::time_t epochTime() const;
		Int64 utcTime() const;

		std::string toString() const;
		std::string toData() const;

		static Timestamp now();
		static Timestamp fromEpochTime(std::time_t t);
		static Timestamp fromUtcTime(Int64 val);

		
		static Timestamp invalid()
		{
			return Timestamp();
		}

		bool valid()
		{
			return mValue > 0;
		}

	protected:

		Int64 mValue;
	};

	inline Timestamp addTime(const Timestamp& timestamp, double seconds)
	{
		Int64 delta = static_cast<int64_t>(seconds * Timestamp::TIME_RESOLUTION);
		return Timestamp(timestamp.epochMicroseconds() + delta);
	}

	inline double timeDifference(const Timestamp& high, const Timestamp& low)
	{
		Int64 diff = high.epochMicroseconds() - low.epochMicroseconds();
		return static_cast<double>(diff) / Timestamp::TIME_RESOLUTION;
	}

}
#endif
