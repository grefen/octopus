#ifndef TIMER_H
#define TIMER_H

#include "Base.h"
#include "Timestamp.h"

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
		{}
			
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


	private:

		
		Timestamp           mExpiration;
		const double        mInterval;
		const bool          mRepeat;
		const TimerCallback mCallback;

	};

}

#endif

