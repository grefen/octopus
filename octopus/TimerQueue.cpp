#include "TimerQueue.h"
#include "Log.h"
#include "Reactor.h"
#include "Timer.h"
#include "TimerId.h"
#include "Timestamp.h"

#include <sys/timerfd.h>

namespace Octopus {


	struct timespec howMuchTimeFromNow(Timestamp when)
	{
		int64_t microseconds = when.epochMicroseconds()
			- Timestamp::now().epochMicroseconds();
		if (microseconds < 100)
		{
			microseconds = 100;
		}
		struct timespec ts;
		ts.tv_sec = static_cast<time_t>(
			microseconds / Timestamp::TIME_RESOLUTION);
		ts.tv_nsec = static_cast<long>(
			(microseconds % Timestamp::TIME_RESOLUTION) * 1000);
		return ts;
	}

	int createTimerfd()
	{
		int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
			TFD_NONBLOCK | TFD_CLOEXEC);
		if (timerfd < 0)
		{
			LOG_SYSFATAL << "Failed in timerfd_create";
		}
		return timerfd;
	}

	void readTimerfd(int timerfd, Timestamp now)
	{
		uint64_t howmany;
		ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
		LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
		if (n != sizeof howmany)
		{
			LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
		}
	}

	void resetTimerfd(int timerfd, Timestamp expiration)
	{
		// wake up loop by timerfd_settime()
		struct itimerspec newValue;
		struct itimerspec oldValue;
		bzero(&newValue, sizeof newValue);
		bzero(&oldValue, sizeof oldValue);
		newValue.it_value = howMuchTimeFromNow(expiration);
		int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
		if (ret)
		{
			LOG_SYSERR << "timerfd_settime()";
		}
	}

	namespace Core {

		TimerQueue::TimerQueue(Reactor* reactor)
			:mpReactor(reactor)
			, mtimerFd(createTimerfd())
			, mtimerFdHander(reactor, mtimerFd)
			, mtimerSet()
			, mcallingExpiredTimers(false)
		{
			mtimerFdHander.setReadCallback(std::bind(&TimerQueue::handleRead, this));
			mtimerFdHander.enableRead(true);
		}


		TimerQueue::~TimerQueue()
		{
			mtimerFdHander.disableAll();
			mtimerFdHander.remove();

			::close(mtimerFd);

			for (TimerSet::iterator it = mtimerSet.begin(); it != mtimerSet.end(); ++it)
			{
				delete it->second;
			}			
		}

		TimerId TimerQueue::addTimer(const TimerCallback& cb,
			Timestamp when,
			double interval)
		{
			Timer* timer = new Timer(when, interval, cb);

			mpReactor->queuePending(std::bind(&TimerQueue::addTimerInReactor, this, timer));

			return TimerId(timer, timer->sequence());
		}

		void TimerQueue::cancel(TimerId timerId)
		{
			//把取消动作放到reactor中排队执行
			mpReactor->queuePending(
				std::bind(&TimerQueue::cancelInReactor, this, timerId));
		}
		void TimerQueue::handleRead()
		{
			
			Timestamp now(Timestamp::now());

			//read timefd
			readTimerfd(mtimerFd, now);

			//获取过期时间
			std::vector<Entry> expired = getExpired(now);

			mcallingExpiredTimers = true;
			cancelingTimers.clear();
			// safe to callback outside critical section
			for (std::vector<Entry>::iterator it = expired.begin();
				it != expired.end(); ++it)
			{
				it->second->run();
			}
			mcallingExpiredTimers = false;

			reset(expired, now);
		}

		void TimerQueue::addTimerInReactor(Timer* timer)
		{
			
			bool earliestChanged = insert(timer);
			//如果这个timer是set中第一个定时器，则reset timerfd，这是poll中最先触发的定时器
			if (earliestChanged)
			{
				resetTimerfd(mtimerFd, timer->expiration());
			}
		}

		void TimerQueue::cancelInReactor(TimerId timerId)
		{			
			assert(mtimerSet.size() == mactiveTimers.size());

			ActiveTimer timer(timerId.getTimer(), timerId.getSequence());
			ActiveTimerSet::iterator it = mactiveTimers.find(timer);
			if (it != mactiveTimers.end())
			{
				//删除两个set中的定时器
				size_t n = mtimerSet.erase(Entry(it->first->expiration(), it->first));
				assert(n == 1); (void)n;
				delete it->first; 
				mactiveTimers.erase(it);
			}
			else if (mcallingExpiredTimers)
			{
				//如果定时器正在使用，则先放到取消列表中
				cancelingTimers.insert(timer);
			}
			assert(mtimerSet.size() == mactiveTimers.size());
		}

		std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
		{
			assert(mtimerSet.size() == mactiveTimers.size());

			std::vector<Entry> expired;
			Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));

			//查找过期定时器
			TimerSet::iterator end = mtimerSet.lower_bound(sentry);
			assert(end == mtimerSet.end() || now < end->first);

			//把过期定时器copy到expired列表中
			std::copy(mtimerSet.begin(), end, back_inserter(expired));
			mtimerSet.erase(mtimerSet.begin(), end);

			//同时删除活动列表中的定时器
			for (std::vector<Entry>::iterator it = expired.begin();
				it != expired.end(); ++it)
			{
				ActiveTimer timer(it->second, it->second->sequence());
				size_t n = mactiveTimers.erase(timer);
				assert(n == 1); (void)n;
			}

			assert(mtimerSet.size() == mactiveTimers.size());

			//这个里面的Timer是new出来的，在reset中释放内存
			return expired;
		}

		void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
		{
			Timestamp nextExpire;

			for (std::vector<Entry>::const_iterator it = expired.begin();
				it != expired.end(); ++it)
			{
				//如果expired中的时间是周期性质的，并且没有被取消，则更新Timer并且重新插入到set中
				ActiveTimer timer(it->second, it->second->sequence());
				if (it->second->repeat()
					&& cancelingTimers.find(timer) == cancelingTimers.end())
				{
					it->second->restart(now);
					insert(it->second);
				}
				else//否则直接删除掉
				{
					
					delete it->second; 
				}
			}

			//获取最近过期时间
			if (!mtimerSet.empty())
			{
				nextExpire = mtimerSet.begin()->second->expiration();
			}
			//如果这个时间是valid，则重新设定到期时间，这样poll中才能触发最近过期时间事件
			if (nextExpire.valid())
			{
				resetTimerfd(mtimerFd, nextExpire);
			}
		}

		bool TimerQueue::insert(Timer* timer)
		{
			//插入到两个set中
			assert(mtimerSet.size() == mactiveTimers.size());
			bool earliestChanged = false;
			Timestamp when = timer->expiration();
			TimerSet::iterator it = mtimerSet.begin();
			if (it == mtimerSet.end() || when < it->first)
			{
				earliestChanged = true;
			}
			{
				std::pair<TimerSet::iterator, bool> result
					= mtimerSet.insert(Entry(when, timer));
				assert(result.second); (void)result;
			}
			{
				std::pair<ActiveTimerSet::iterator, bool> result
					= mactiveTimers.insert(ActiveTimer(timer, timer->sequence()));
				assert(result.second); (void)result;
			}

			assert(mtimerSet.size() == mactiveTimers.size());
			return earliestChanged;
		}
	}

}
