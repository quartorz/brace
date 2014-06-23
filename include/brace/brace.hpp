#pragma once

#include <boost/optional/optional.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/range/iterator_range_io.hpp>

#include <boost/lexical_cast.hpp>

#include <string>
#include <tuple>
#include <utility>

#include "detail/ssl_handler.hpp"

class oauth_handler{
	std::string key, secret;

public:
	oauth_handler(const std::string &key, const std::string &secret)
		: key(key)
		, secret(secret)
	{
	}

public:
	// returns (request token, request token secret) or boost::none
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

		auto &token = detail::parse_url_query(content, "oauth_token=");
		if(!token)
			return boost::none;

		auto &secret = detail::parse_url_query(content, "oauth_token_secret=");
		if(!secret)
			return boost::none;

		return std::make_tuple(*token, *secret);
	}
};
