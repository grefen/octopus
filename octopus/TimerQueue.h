#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "Base.h"
#include "Timer.h"
#include "TimerId.h"
#include "EventHandler.h"

namespace Octopus {

	class Reactor;
	class Timer;
	class TimerId;

	namespace Core {



		class TimerQueue : public Noncopyable
		{
		public:

			typedef std::pair<Timestamp, Timer*> Entry;
			typedef std::set<Entry>              TimerSet;
			typedef std::pair<Timer*, UInt64>     ActiveTimer;
			typedef std::set<ActiveTimer>       ActiveTimerSet;

			TimerQueue(Reactor* reactor);
			~TimerQueue();

			TimerId addTimer(const TimerCallback& cb,
				Timestamp when,
				double interval);
			void cancel(TimerId id);

			void handleRead();

			void addTimerInReactor(Timer* timer);
			void cancelInReactor(TimerId timerId);

			std::vector<Entry> getExpired(Timestamp now);
			void reset(const std::vector<Entry>& expired, Timestamp now);

			bool insert(Timer* timer);
		private:

			Reactor*     mpReactor;

			//
			const int    mtimerFd;
			EventHandler mtimerFdHander;

			//采用红黑树保存定时器
			TimerSet     mtimerSet;


			ActiveTimerSet   mactiveTimers;
			bool             mcallingExpiredTimers;
			ActiveTimerSet   cancelingTimers;

		};

	}

}


#endif

