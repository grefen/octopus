#ifndef TIMERID_H
#define TIMERID_H

#include "Base.h"
#include "Timestamp.h"
#include "Timer.h"

namespace Octopus {

	//采用Timer地址作为Id
	class TimerId : public Copyable
	{
	public:
		TimerId()
			:mTimer(0)
			, mSequence(0)
		{
		}
		TimerId(Timer* timer, UInt64 seq)
			:mTimer(timer)
			, mSequence(seq)
		{

		}
		~TimerId()
		{

		}

		Timer* getTimer()
		{
			return mTimer;
		}
		UInt64 getSequence()
		{
			return mSequence;
		}
	private:

		Timer* mTimer;
		UInt64 mSequence;//防止定制器冲突，两个定时器地址可能相同，用一个序列号来区分
	};

}

#endif

