#include "Reactor.h"
#include "EventHandler.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/eventfd.h>

#include <fcntl.h>
#include <stdio.h> 
#include <strings.h>  
#include <sys/socket.h>
#include <unistd.h>

#include "Poller.h"
#include "Log.h"
#include "EventHandler.h"


namespace Octopus {

	namespace Core {

		static int createEventfd()
		{
			int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
			if (evtfd < 0)
			{
				LOG_SYSERR << "Failed in create eventfd";
				abort();
			}
			return evtfd;
		}

		static Poller* createPoller(Reactor* reactor)
		{
			return new Poller(reactor);
		}

		static EventHandler* createEventHandler(Reactor* reactor, int wakeupFd)
		{
			return new EventHandler(reactor, wakeupFd);
		}

#pragma GCC diagnostic ignored "-Wold-style-cast"
		class IgnorePipeSignal
		{
		public:
			IgnorePipeSignal()
			{
				::signal(SIGPIPE, SIG_IGN);
				// LOG_TRACE << "Ignore SIGPIPE";
			}
		};
#pragma GCC diagnostic error "-Wold-style-cast"

		IgnorePipeSignal initObj;

		Reactor::Reactor()
			: mloop(false)
			, mquit(false)
			, meventHanding(false)
			, mrunPendingFunctors(false)
			, miteration(0)
			, mPoller(createPoller(this))
			, mtimerQueue(new TimerQueue(this))
			, mwakeupFd(createEventfd())
			, mWakeupEventHandler(createEventHandler(this, mwakeupFd))
			, mpCurrentActiveEventHandler(NULL)
		{
			mWakeupEventHandler->setReadCallback(std::bind(&Reactor::handleWakeup, this));
			mWakeupEventHandler->enableRead(true);
		}


		Reactor::~Reactor()
		{
			mWakeupEventHandler->disableAll();
			mWakeupEventHandler->remove();

			::close(mwakeupFd);
		}

		void Reactor::Loop()
		{
			mloop = true;
			mquit = false;

			LOG_TRACE << "Reactor start running";

			while (!mquit)
			{
				mActiveEventHandlers.clear();

				miteration++;

				mpollReturnTime = mPoller->poll(POLL_TIMEMS, &mActiveEventHandlers);

				meventHanding = true;

				for (auto it = mActiveEventHandlers.begin(); it != mActiveEventHandlers.end(); ++it)
				{
					mpCurrentActiveEventHandler = *it;
					mpCurrentActiveEventHandler->handleEvent(mpollReturnTime);
				}

				mpCurrentActiveEventHandler = NULL;

				meventHanding = false;

				handlePendingFunctors();
			}

			

			mloop = false;
		}

		void Reactor::removeHandler(EventHandler* handler)
		{
			mPoller->removeEventHandler(handler);
		}

		void Reactor::updateHandler(EventHandler* handler)
		{
			mPoller->updateEventHandler(handler);
		}

		void Reactor::handleWakeup()
		{
			UInt64 one = 1;
			ssize_t n = ::write(mwakeupFd, &one, sizeof one);
			if (n != sizeof one)
			{
				LOG_ERROR << "Reactor::wakeup() writes " << n << " bytes";
			}
		}

		void Reactor::handlePendingFunctors()
		{
			std::vector<PendingFunctor> functors;
			mrunPendingFunctors = true;

			functors.swap(mPendingFunctors);			

			for (size_t i = 0; i < functors.size(); ++i)
			{
				functors[i]();
			}
			mrunPendingFunctors = false;
		}

		void Reactor::quit()
		{
			mquit = true;

			wakeup();
		}

		void Reactor::wakeup()
		{
			UInt64 one = 1;
			ssize_t n = ::write(mwakeupFd, &one, sizeof one);
			if (n != sizeof one)
			{
				LOG_ERROR << "Reactor::wakeup() writes ";
			}
		}

		void Reactor::queuePending(PendingFunctor functor)
		{
			mPendingFunctors.push_back(functor);

			if (mrunPendingFunctors)
			{
				wakeup();
			}
		}
		void Reactor::runInReactor(const PendingFunctor& cb)
		{
			cb();
		}

		TimerId Reactor::setTimer(const Timestamp& time, const TimerCallback& cb)
		{
			return mtimerQueue->addTimer(cb, time, 0);
		}
		TimerId Reactor::setDelayTimer(double delay, const TimerCallback& cb)
		{
			Timestamp time(addTime(Timestamp::now(), delay));
			return setTimer(time, cb);
		}
		TimerId Reactor::setTickTimer(double interval, const TimerCallback& cb)
		{
			Timestamp time(addTime(Timestamp::now(), interval));
			return mtimerQueue->addTimer(cb, time, interval);
		}
		void    Reactor::cancelTimer(TimerId timerId)
		{
			return mtimerQueue->cancel(timerId);
		}

		bool Reactor::hasHandler(EventHandler* handler)
		{
			return mPoller->hasEventHandler(handler);
		}

	}
}
