#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "Base.h"

namespace Octopus {

	class Exception : public std::exception
	{
	public:
		Exception(const std::string& msg, int code = 0);

		Exception(const std::string& msg, const std::string& arg, int code = 0);

		Exception(const Exception& exc);

		~Exception();

		Exception& operator = (const Exception& exc);

		virtual const char* name() const  throw();

		virtual const char* className() const  throw();

		virtual const char* what() const throw();

		const std::string& message() const;

		int code() const;

	protected:
		Exception(int code = 0);

		void message(const std::string& msg);

	private:
		std::string mmsg;
		int			mcode;
	};

#define DECLARE_EXCEPTIOIN_CODE(CLASS, BASE, CODE) \
      class CLASS : public BASE                    \
      {                                            \
      public:                                      \
	    CLASS(int code = CODE);                      \
		CLASS(const std::string& msg, int code = CODE); \
        CLASS(const std::string& msg, const std::string& arg, int code = CODE);		\
		CLASS(const CLASS& exc);                       \
		~CLASS();                                   \
		CLASS& operator = (const CLASS& exc);       \
		const char*  name() const  throw();                  \
		const char* className() const  throw();              \
	};

#define DECLARE_EXCEPTION(CLASS, BASE)    DECLARE_EXCEPTIOIN_CODE(CLASS, BASE, 0)
        

#define IMPLEMENT_EXCEPTION(CLASS, BASE, NAME)        \
        CLASS::CLASS(int code) : BASE(code)           \
        {}                                            \
        CLASS::CLASS(const std::string& msg, int code):BASE(msg,code)\
        {}                                            \
	    CLASS::CLASS(const std::string& msg, const std::string& arg, int code): BASE(msg, arg, code)\
        {}                                            \
        CLASS::CLASS(const CLASS& exc) : BASE(exc)    \
        {}                                            \
        CLASS::~CLASS()                               \
        {}                                            \
        CLASS& CLASS::operator = (const CLASS& exc)   \
        {                                             \
             BASE::operator = (exc);                  \
	         return *this;                            \
	    }                                             \
	    const char* CLASS::name() const   throw()             \
	    {                                             \
		    return NAME;                               \
		}                                              \
		const char* CLASS::className() const   throw()         \
		{                                              \
			return typeid(*this).name();               \
		}                                       		


		DECLARE_EXCEPTION(LogicException, Exception)


		DECLARE_EXCEPTION(RuntimeException, Exception)
		DECLARE_EXCEPTION(HostNotFoundException, RuntimeException)
		DECLARE_EXCEPTION(DNSException, RuntimeException) 
		DECLARE_EXCEPTION(InvalidArgumentException, RuntimeException)
		DECLARE_EXCEPTION(SystemException, RuntimeException)

		
}
#endif
