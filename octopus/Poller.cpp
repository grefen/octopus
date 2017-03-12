#include "Poller.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

#include "Log.h"
#include "EventHandler.h"


namespace Octopus {

	namespace Core {

		Poller::Poller(Reactor* reactor)
			:mReactor(reactor)
			, mEpollfd(::epoll_create1(EPOLL_CLOEXEC))
			, mEventsList(INIT_EVENTLIST_SIZE)
		{
			if (mEpollfd < 0)
			{
				LOG_SYSFATAL << "Create epoller failed";
			}
		}


		Poller::~Poller()
		{
			::close(mEpollfd);
		}

		Timestamp Poller::poll(int timems, EventHandlerList* activeList)
		{
			LOG_TRACE << "total count fd" << mMapEventHandler.size();

			int num = ::epoll_wait(mEpollfd,
				&*mEventsList.begin(),
				static_cast<int>(mEventsList.size()),
				timems);
			int savedErrno = errno;

			Timestamp now(Timestamp::now());
			if (num > 0)
			{
				LOG_TRACE << num << " events happended";

				extractActiveEventHandlers(num, activeList);
				if (static_cast<size_t>(num) == mEventsList.size())
				{
					mEventsList.resize(mEventsList.size() * 2);
				}

			}
			else if (num == 0)
			{
				LOG_TRACE << "nothing happended";
			}
			else
			{
				// error happens, log uncommon ones
				if (savedErrno != EINTR)
				{
					errno = savedErrno;
					LOG_SYSERR << "EPollPoller::poll()";
				}
			}
			return now;
		}

		void Poller::updateEventHandler(EventHandler* handler)
		{

			const int index = handler->getIndex();

			LOG_TRACE << "fd = " << handler->fd()
				<< " events = " << handler->getEvents() << " index = " << index;

			if (index == OP_INDEX_NEW || index == OP_INDEX_DEL)
			{
				
				int fd = handler->fd();
				if (index == OP_INDEX_NEW)
				{
					assert(mMapEventHandler.find(fd) == mMapEventHandler.end());
					mMapEventHandler[fd] = handler;
				}
				else 
				{
					assert(mMapEventHandler.find(fd) != mMapEventHandler.end());
					assert(mMapEventHandler[fd] == handler);
				}

				handler->setIndex(OP_INDEX_ADD);
				update(EPOLL_CTL_ADD, handler);
			}
			else
			{
				
				int fd = handler->fd();
				(void)fd;

				assert(mMapEventHandler.find(fd) != mMapEventHandler.end());
				assert(mMapEventHandler[fd] == handler);
				assert(index == OP_INDEX_ADD);

				if (handler->isNoneEvent())
				{
					update(EPOLL_CTL_DEL, handler);
					handler->setIndex(OP_INDEX_DEL);
				}
				else
				{
					update(EPOLL_CTL_MOD, handler);
				}
			}
		}

		void Poller::removeEventHandler(EventHandler* handler)
		{

			int fd = handler->fd();

			LOG_TRACE << "fd = " << fd;

			assert(mMapEventHandler.find(fd) != mMapEventHandler.end());
			assert(mMapEventHandler[fd] == handler);
			
			int index = handler->getIndex();
			assert(index == OP_INDEX_ADD || index == OP_INDEX_DEL);
			size_t n = mMapEventHandler.erase(fd);
			(void)n;
			assert(n == 1);

			if (index == OP_INDEX_ADD)
			{
				update(EPOLL_CTL_DEL, handler);
			}
			handler->setIndex(OP_INDEX_NEW);
		}

		bool Poller::hasEventHandler(EventHandler* handler)
		{
			MapEventHandler::const_iterator it = mMapEventHandler.find(handler->fd());
			return it != mMapEventHandler.end() && it->second == handler;
		}

		void Poller::extractActiveEventHandlers(int num, EventHandlerList* activeList)
		{
			for (int i = 0; i < num; ++i)
			{
				EventHandler* handler = static_cast<EventHandler*>(mEventsList[i].data.ptr);

				handler->setReadEvents(mEventsList[i].events);
				activeList->push_back(handler);
			}
		}

		void Poller::update(int operation, EventHandler* handler)
		{
			struct epoll_event event;
			bzero(&event, sizeof event);
			event.events = handler->getEvents();
			event.data.ptr = handler;
			int fd = handler->fd();
				
			if (::epoll_ctl(mEpollfd, operation, fd, &event) < 0)
			{
				if (operation == EPOLL_CTL_DEL)
				{
					LOG_SYSERR << "epoll_ctl op = DEL" << " fd =" << fd;
				}
				else
				{
					LOG_SYSFATAL << "epoll_ctl op = OTHER" << " fd =" << fd;
				}
			}
		}

	}
}
