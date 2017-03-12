#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "Base.h"
#include "Timestamp.h"
#include <poll.h>

namespace Octopus {

	namespace Core {

		class Reactor;

		class EventHandler : public Noncopyable
		{
		public:

			typedef std::function<void()> CallBack;
			typedef std::function<void(Timestamp)> ReadCallback;

			const int NONE_EVENT = 0;
			const int READ_EVENT = POLLIN | POLLPRI;
			const int WRITE_EVENT = POLLOUT;

			EventHandler(Reactor* rector, int fd);
			~EventHandler();

			void handleEvent(Timestamp receiveTime);

			void setReadCallback(const ReadCallback&);
			void setWriteCallback(const CallBack&);
			void setCloseCallback(const CallBack&);
			void setErrorCallback(const CallBack&);

			int fd();
			int getEvents();
			void setReadEvents(int events);

			bool isNoneEvent() const { return mevents == NONE_EVENT; }

			void enableRead(bool enable);
			void enableWrite(bool enable);
			void disableAll() { mevents = NONE_EVENT; update(); }

			bool isEnableRead();
			bool isEnalbeWrite();

			int  getIndex();
			void setIndex(int idx);

			Reactor* getReactor();

			void remove();
			void update();
			
			std::string reventsToString() const;
			std::string eventsToString() const;

			bool isWriting() const { return mevents & WRITE_EVENT; }
			bool isReading() const { return mevents & READ_EVENT; }
		private:
			static std::string eventsToString(int fd, int ev);
		private:

			Reactor* mRector;
			const int mfd;

			ReadCallback  mreadCallback;
			CallBack      mwriteCallback;
			CallBack      mcloseCallback;
			CallBack      merrorCallback;

			int        mevents;
			int        mrevents; 
			int        mindex; 

			bool       meventHandling;
		};
	}
}

#endif

