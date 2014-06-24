#pragma once

#include <stdexcept>
#include <string>

namespace brace{

	class brace_exception: public std::exception{
		std::string message;

	public:
#if !defined(_MSC_VER) || _MSC_VER >= 1800
		brace_exception() = default;
		brace_exception(const brace_exception&) = default;
#endif
#if !defined(_MSC_VER) || _MSC_VER > 1800
		brace_exception(brace_exception &&) = default;
#endif

		brace_exception(const std::string &m):message(m) {}
		brace_exception(std::string &&m):message(m) {}

#if !defined(_MSC_VER) || _MSC_VER >= 1800
		brace_exception &operator=(const brace_exception&) = default;
#endif
#if !defined(_MSC_VER) || _MSC_VER > 1800
		brace_exception &operator=(brace_exception&&) = default;
#endif

		const char *what() const override
		{
			return message.c_str();
		}
	};

}