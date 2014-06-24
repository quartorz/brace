#pragma once

#include <boost/optional/optional.hpp>

#include <boost/lexical_cast.hpp>

#include <string>
#include <tuple>
#include <utility>

#include "detail/ssl_handler.hpp"
#include "detail/functions.hpp"

namespace brace{

	class oauth_handler{
		std::pair<std::string, std::string> consumer, access;

	public:
		oauth_handler(const std::string &consumer_key, const std::string &consumer_secret)
			: consumer(consumer_key, consumer_secret)
		{
		}
		oauth_handler(std::string &&consumer_key, std::string &&consumer_secret)
			: consumer(std::move(consumer_key), std::move(consumer_secret))
		{
		}

		void set_access_token(const std::string &token, const std::string &secret)
		{
			access = {token, secret};
		}
		void set_access_token(std::string &&token, std::string &&secret)
		{
			access = {std::move(token), std::move(secret)};
		}

		boost::optional<std::string> get_authorize_url();

		// returns response from Twitter or boost::none
		boost::optional<std::string> set_pin(const std::string &pin);

	private:
		// returns (request token, request token secret) or boost::none
		boost::optional<std::pair<std::string, std::string>> get_request_token();
	};

	inline boost::optional<std::string> oauth_handler::get_authorize_url()
	{
		auto req = get_request_token();
		if(!req)
			return boost::none;

		access = std::make_pair(std::get<0>(*req), std::move(std::get<1>(*req)));

		std::string &token = std::get<0>(*req);
		token.insert(0, "https://api.twitter.com/oauth/authorize?oauth_token=");

		return token;
	}

	// returns response from Twitter or boost::none
	inline boost::optional<std::string> oauth_handler::set_pin(const std::string &pin)
	{
		static const char *host = "api.twitter.com";
		static const char *endpoint = "/oauth/access_token";

		if(std::get<0>(access).length() == 0 || std::get<1>(access).length() == 0)
			return boost::none;

		detail::ssl_handler handler(host, endpoint, detail::http_method::POST);

		if(!handler.resolve_host()){
			return boost::none;
		}

		auto result = handler.connect(consumer, access, {{"oauth_verifier", pin}});

		if(std::get<0>(result) != 200)
			return boost::none;

		std::cout << std::get<1>(result) << std::endl;

		return std::move(std::get<1>(result));
	}

	// returns (request token, request token secret) or boost::none
	inline boost::optional<std::pair<std::string, std::string>> oauth_handler::get_request_token()
	{
		static const char *host = "api.twitter.com";
		static const char *endpoint = "/oauth/request_token";

		detail::ssl_handler handler(host, endpoint, detail::http_method::POST);

		if(!handler.resolve_host()){
			return boost::none;
		}

		auto result = handler.connect(consumer, {});

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

		return std::make_pair(std::move(*token), std::move(*secret));
	}

}
