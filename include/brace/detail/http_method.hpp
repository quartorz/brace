#pragma once

namespace detail{
	enum class http_method: int{
		GET,
		POST,
	};

	const char *to_string(http_method m)
	{
		static const char *table[] ={
			"GET",
			"POST",
		};
		return table[static_cast<int>(m)];
	}
}
