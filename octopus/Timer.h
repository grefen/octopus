#ifndef TIMER_H
#define TIMER_H

#include "Base.h"
#include "Timestamp.h"
#include <atomic>

namespace Octopus {

	typedef std::function<void()> TimerCallback;

	class Timer : Noncopyable
	{
	public:
		Timer(Timestamp when, double interval, const TimerCallback& cb)
			:mExpiration(when)
			, mInterval(interval)
			, mRepeat(interval > 0.0)
			, mCallback(cb)
			
		{
			mSequence++;
		}
			
		~Timer() {}

		void run()
		{
			mCallback();
		}

		Timestamp expiration()
		{
			return mExpiration;
		}

		bool repeat()
		{
			return mRepeat;
		}

		void restart(Timestamp now);

		UInt64 sequence()
		{
			return mSequence;
		}

		static UInt64 countCreated()
		{
			return mSequence;
		}
	private:

		
		Timestamp           mExpiration;
		const double        mInterval;
		const bool          mRepeat;
		const TimerCallback mCallback;

		static std::atomic_ullong   mSequence;
	};

}

#endif

