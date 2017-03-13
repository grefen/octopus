#include "EventHandler.h"
#include "Reactor.h"
#include "Log.h"

namespace Octopus {

	namespace Core {

		EventHandler::EventHandler(Reactor* rector, int fd)
			: mRector(rector)
			, mfd(fd)
			, mevents(NONE_EVENT)
			, mrevents(NONE_EVENT)
			, mindex(-1)
			, meventHandling(false)
			, mownered(false)
		{

		}


		EventHandler::~EventHandler()
		{
		}

		void EventHandler::handleEvent(Timestamp receiveTime)
		{
			if (mownered == false)
				return;

			//·ÀÖ¹TcpConnection¹ýÔçÏú»Ù
			std::shared_ptr<void> spOwner = mowner.lock();
			if (!spOwner) return;

			meventHandling = true;
			LOG_TRACE << reventsToString();

			if ((mrevents & POLLHUP) && !(mrevents & POLLIN))
			{
				if (mcloseCallback) mcloseCallback();
			}

			if (mrevents & POLLNVAL)
			{
				LOG_WARN << "fd = " << mfd << " handle_event() POLLNVAL";
			}

			if (mrevents & (POLLERR | POLLNVAL))
			{
				if (merrorCallback) merrorCallback();
			}
			if (mrevents & (POLLIN | POLLPRI | POLLRDHUP))
			{
				if (mreadCallback) mreadCallback(receiveTime);
			}
			if (mrevents & POLLOUT)
			{
				if (mwriteCallback) mwriteCallback();
			}
			meventHandling = false;
		}

		void EventHandler::setReadCallback(const ReadCallback& callback)
		{
			mreadCallback = callback;
		}
		void EventHandler::setWriteCallback(const CallBack& callback)
		{
			mwriteCallback = callback;
		}
		void EventHandler::setCloseCallback(const CallBack& callback)
		{
			mcloseCallback = callback;
		}
		void EventHandler::setErrorCallback(const CallBack& callback)
		{
			merrorCallback = callback;
		}

		int EventHandler::fd()
		{
			return mfd;
		}
		int EventHandler::getEvents()
		{
			return mevents;
		}
		void EventHandler::setReadEvents(int events)
		{
			mrevents = events;
		}
		void EventHandler::enableRead(bool enable)
		{
			if (enable)
			{
				mevents |= READ_EVENT;
			}
			else
			{
				mevents &= ~READ_EVENT;
			}
		}
		void EventHandler::enableWrite(bool enable)
		{
			if (enable)
			{
				mevents |= WRITE_EVENT;
			}
			else
			{
				mevents &= ~WRITE_EVENT;
			}
		}

		bool EventHandler::isEnableRead()
		{
			return mrevents & READ_EVENT;
		}
		bool EventHandler::isEnalbeWrite()
		{
			return mevents & WRITE_EVENT;
		}
		int  EventHandler::getIndex()
		{
			return mindex;
		}
		void EventHandler::setIndex(int idx)
		{
			mindex = idx;
		}
		Reactor* EventHandler::getReactor()
		{
			return mRector;
		}
		std::string EventHandler::reventsToString() const
		{
			return eventsToString(mfd, mrevents);
		}

		std::string EventHandler::eventsToString() const
		{
			return eventsToString(mfd, mevents);
		}

		std::string EventHandler::eventsToString(int fd, int ev)
		{
			std::ostringstream oss;
			oss << fd << ": ";
			if (ev & POLLIN)
				oss << "IN ";
			if (ev & POLLPRI)
				oss << "PRI ";
			if (ev & POLLOUT)
				oss << "OUT ";
			if (ev & POLLHUP)
				oss << "HUP ";
			if (ev & POLLRDHUP)
				oss << "RDHUP ";
			if (ev & POLLERR)
				oss << "ERR ";
			if (ev & POLLNVAL)
				oss << "NVAL ";

			return oss.str().c_str();
		}

		void EventHandler::remove()
		{
			mRector->removeHandler(this);
		}
		void EventHandler::update()
		{
			mRector->updateHandler(this);
		}

		void EventHandler::bindOwner(const std::shared_ptr<void>& owner)
		{
			mowner = owner;
			mownered = true;
		}

	}
}
