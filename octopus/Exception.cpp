#include"Exception.h"
#include <typeinfo>

namespace Octopus {

	Exception::Exception(int code) : mcode(code)
	{
	}

	Exception::Exception(const std::string& msg, int code) : mmsg(msg),mcode(code)
	{
	}

	Exception::Exception(const std::string& msg, const std::string& arg, int code) : mmsg(msg),mcode(code)
	{
		if (!arg.empty())
		{
			mmsg.append(": ");
			mmsg.append(arg);
		}
	}

	Exception::Exception(const Exception& exc):
		std::exception(exc),
		mmsg(exc.mmsg),
		mcode(exc.mcode)
	{
	}

	Exception::~Exception()
	{
	}

	Exception& Exception::operator=(const Exception& exc)
	{
		if (&exc != this)
		{
			mmsg = exc.mmsg;
			mcode = exc.mcode;
		}

		return *this;
	}

	const char* Exception::name() const  throw()
	{
		return "Exception";
	}

	const char* Exception::className() const  throw()
	{
		return typeid(*this).name();
	}

	const char* Exception::what() const throw()
	{
		return name();
	}

	const std::string& Exception::message() const
	{
		return mmsg;
	}

	void Exception::message(const std::string& msg)
	{
		mmsg = msg;
	}

	int Exception::code() const
	{
		return mcode;
	}


	IMPLEMENT_EXCEPTION(LogicException, Exception, "Logic exception")


	IMPLEMENT_EXCEPTION(RuntimeException, Exception, "Runtime exception")
	IMPLEMENT_EXCEPTION(HostNotFoundException, RuntimeException, "Host not found")
	IMPLEMENT_EXCEPTION(DNSException, RuntimeException, "DNS error")
	IMPLEMENT_EXCEPTION(InvalidArgumentException, RuntimeException, "Invalid argument")
	IMPLEMENT_EXCEPTION(SystemException, RuntimeException, "System exception")
}