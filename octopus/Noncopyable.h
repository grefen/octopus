#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace Octopus {


		class Noncopyable
		{
		protected:

			Noncopyable() = default;
			~Noncopyable() = default;

			Noncopyable(const Noncopyable&) = delete;
			Noncopyable& operator=(const Noncopyable&) = delete;
		};

	
}

#endif

