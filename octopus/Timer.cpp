#include "Timer.h"
#include "Timestamp.h"

namespace Octopus {


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
