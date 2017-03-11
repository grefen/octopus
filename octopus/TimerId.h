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
		{
		}
		~TimerId()
		{

		}

	private:

		Timer* mTimer;
	};

}

#endif

