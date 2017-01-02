#ifndef STRING_H
#define STRING_H

#include "Base.h"

namespace Octopus {

	std::string& trimLeft(std::string& str)
	{
		std::string::const_iterator it = str.begin();
		std::string::const_iterator end = str.end();

		while (it != end && std::isspace(*it)) ++it;

		str.erase(str.begin(), it);

		return str;
	}

	std::string& trimRight(std::string& str)
	{
		int pos = int(str.size()) - 1;
		while (pos >= 0 && std::isspace(str[pos])) --pos;

		str.resize(pos + 1);

		return str;
	}

	std::string& trim(std::string& str)
	{
		int first = 0;
		int last = int(str.size()) - 1;

		while (first <= last && std::isspace(str[first])) ++first;
		while (last >= first && std::isspace(str[last])) --last;

		str.resize(last + 1);
		str.erase(0, first);

		return str;
	}

	std::string& toUpper(std::string& str)
	{
		std::string::iterator it = str.begin();
		std::string::iterator end = str.end();

		while (it != end) {
			*it = static_cast<std::string::value_type>(std::toupper(*it));
			++it;
		}

		return str;
	}

	std::string& toLower(std::string& str)
	{
		std::string::iterator  it = str.begin();
		std::string::iterator  end = str.end();

		while (it != end) {
			*it = static_cast<std::string::value_type>(std::tolower(*it));
			++it;
		}

		return str;
	}

	template<typename T>
	std::string toString(T i)
	{
		std::stringstream s;
		s << i;

		return s.str();
	}




}
#endif
