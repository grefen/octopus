#ifndef RECTOR_H
#define RECTOR_H


#include "Base.h"
#include "Reactor.h"
#include "Poller.h"
#include "Timestamp.h"
#include "TimerId.h"
#include "TimerQueue.h"

namespace Octopus {

	namespace Core {

		class EventHandler;
		class Poller;
		class Reactor : public Noncopyable
		{
		public:

			typedef std::function<void()> PendingFunctor;

			static const int POLL_TIMEMS = 10000;

			Reactor();
			~Reactor();

			void Loop();

			void removeHandler(EventHandler*);
			void updateHandler(EventHandler*);
			bool hasHandler(EventHandler*);

			void quit();
			void wakeup();

			void queuePending(PendingFunctor functor);

			
			TimerId setTimer(const Timestamp& time, const TimerCallback& cb);
			TimerId setDelayTimer(double delay, const TimerCallback& cb);
			TimerId setTickTimer(double interval, const TimerCallback& cb);
			void    cancelTimer(TimerId timerId);

		protected:

			void handleWakeup();
			void handlePendingFunctors();
		private:
			typedef std::vector<EventHandler*> EventHandlerList;

			bool  mloop;
			bool  mquit;
			bool  meventHanding;
			bool  mrunPendingFunctors;

			UInt64 miteration;

			Timestamp mpollReturnTime;

			std::unique_ptr<Poller> mPoller;

			//timerqueue
			std::unique_ptr<TimerQueue> mtimerQueue;

			//唤醒poller，用于统一事件源，如信号
			int mwakeupFd;
			std::unique_ptr<EventHandler> mWakeupEventHandler;

			//context;

			//每次从poll中获取当前要处理的handler
			EventHandlerList mActiveEventHandlers;
			EventHandler     *mpCurrentActiveEventHandler;

			std::vector<PendingFunctor> mPendingFunctors;

		};

	}
}

#endif

