#ifndef RECTOR_H
#define RECTOR_H


#include "Base.h"
#include "Reactor.h"
#include "Poller.h"
#include "Timestamp.h"

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

			void quit();
			void wakeup();

			void queuePending(PendingFunctor functor);
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

			int mwakeupFd;

			std::unique_ptr<EventHandler> mWakeupEventHandler;

			//context;

			EventHandlerList mActiveEventHandlers;
			EventHandler     *mpCurrentActiveEventHandler;

			std::vector<PendingFunctor> mPendingFunctors;

		};

	}
}

#endif

