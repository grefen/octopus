#include "Timer.h"
#include "Timestamp.h"

namespace Octopus {

	std::atomic_ullong Timer::mSequence(0);

	void Timer::restart(Timestamp now)
	{
		if (mRepeat)
		{
			mExpiration = addTime(now, mInterval);
		}
		else
		{
			mExpiration = Timestamp::invalid();
		}
	}

}
