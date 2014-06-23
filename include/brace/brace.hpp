#pragma once

#include "detail/ssl_handler.hpp"

#include <boost/optional/optional.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/range/iterator_range_io.hpp>

#include <boost/lexical_cast.hpp>

#include <string>
#include <tuple>
#include <utility>

class oauth_handler{
	std::string key, secret;

public:
	oauth_handler(const std::string &key, const std::string &secret)
		: key(key)
		, secret(secret)
	{
	}

public:
	boost::optional<std::tuple<std::string, std::string>> get_request_token()
	{
		static const char *host = "api.twitter.com";
		static const char *endpoint = "/oauth/request_token";

		detail::ssl_handler handler(host, endpoint, detail::http_method::POST);

		if(!handler.resolve_host()){
			return boost::none;
		}

		auto result = handler.connect({key, secret}, {});

		std::cout << std::get<0>(result) << std::endl << std::get<1>(result) << std::endl;

		if(std::get<0>(result) != 200)
			return boost::none;

		std::string &content = std::get<1>(result);

		std::string::size_type key_begin, key_end, sec_begin, sec_end;

		key_begin = content.find("oauth_token=");
		if(key_begin == std::string::npos)
			return boost::none;
		key_begin += 12;

		key_end = content.find('&', key_begin);
		if(key_end == std::string::npos)
			key_end = content.length() - 1;

		sec_begin = content.find("oauth_token_secret=");
		if(sec_begin == std::string::npos)
			return boost::none;
		sec_begin += 19;

		sec_end = content.find('&', sec_begin);
		if(sec_end == std::string::npos)
			sec_end = content.length() - 1;

		return std::make_tuple(
			boost::lexical_cast<std::string>(
				boost::make_iterator_range(&content[key_begin], &content[key_end])),
			boost::lexical_cast<std::string>(
				boost::make_iterator_range(&content[sec_begin], &content[sec_end])));
	}
};
