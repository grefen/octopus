#ifndef POLLER_H
#define POLLER_H

#include "Base.h"
#include "Timestamp.h"
#include "Reactor.h"

struct epoll_event;

namespace Octopus {

	namespace Core {

		class EventHandler;
		class Reactor;

		class Poller : public Noncopyable
		{
		public:
			typedef std::vector<EventHandler*> EventHandlerList;

			static const int INIT_EVENTLIST_SIZE = 16;

			static const int OP_INDEX_NEW = -1;
			static const int OP_INDEX_ADD = 1;
			static const int OP_INDEX_DEL = 2;

			Poller(Reactor* reactor);
			~Poller();

			Timestamp poll(int timems, EventHandlerList* activeList);

			void updateEventHandler(EventHandler* handler);
			void removeEventHandler(EventHandler* handler);
		protected:

			void extractActiveEventHandlers(int num, EventHandlerList* activeList);
			void update(int operation, EventHandler* handler);
		private:
			typedef std::map<int, EventHandler*> MapEventHandler;
			typedef std::vector<struct epoll_event> EventList;

			MapEventHandler mMapEventHandler;

			Reactor*      mReactor;

			int           mEpollfd;
			EventList     mEventsList;


		};

	}
}

#endif

