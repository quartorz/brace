#pragma once

#include "detail/ssl_handler.hpp"

#include <boost/optional/optional.hpp>

#include <string>
#include <tuple>
#include <utility>

const char *get_pointer(const char *s)
{
	return s;
}

const char *get_pointer(const std::string &s)
{
	return s.c_str();
}

template <class T>
class get_impl{
public:
	using first_type = typename std::add_reference<typename std::add_const<typename T::value_type>::type>::type;
	using second_type = typename std::add_reference<typename std::add_const<typename T::value_type>::type>::type;
	second_type get_first(const T &list)
	{
		return *list.begin();
	}
	first_type get_second(const T &list)
	{
		return *(list.begin() + 1);
	}
};

template <class T1, class T2>
class get_impl<std::tuple<T1, T2>>{
public:
	using first_type = typename std::add_reference<typename std::add_const<T1>::type>::type;
	using second_type = typename std::add_reference<typename std::add_const<T1>::type>::type;
	first_type get_first(const std::tuple<T1, T2> &t)
	{
		return std::get<0>(t);
	}
	second_type get_second(const std::tuple<T1, T2> &t)
	{
		return std::get<1>(t);
	}
};

template <class T1, class T2>
class get_impl<std::pair<T1, T2>>{
public:
	using first_type = typename std::add_reference<typename std::add_const<T1>::type>::type;
	using second_type = typename std::add_reference<typename std::add_const<T1>::type>::type;
	first_type get_first(const std::pair<T1, T2>  &p)
	{
		return std::get<0>(t);
	}
	second_type get_second(const std::pair<T1, T2> &t)
	{
		return std::get<1>(t);
	}
};

template <class T>
typename get_impl<T>::first_type get_first(const T &o)
{
	return get_impl<T>().get_fisrt(o);
}

template <class T>
typename get_impl<T>::second_type get_second(const T &o)
{
	return get_impl<T>().get_second(o);
}

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

#if 1
		detail::ssl_handler handler(host, endpoint, detail::http_method::POST);

		if(!handler.resolve_host()){
			return boost::none;
		}

		auto result = handler.connect({key, secret}, {});

		std::cout << std::get<0>(result) << std::endl << std::get<1>(result) << std::endl;

		return std::make_tuple(std::get<1>(result), "");
#else
		namespace asio = boost::asio;
		namespace ssl = asio::ssl;
		using tcp = asio::ip::tcp;

		asio::io_service service;

		ssl::context context(service, ssl::context::sslv3_client);
		ssl::stream<tcp::socket> ssl_stream(service, context);
		ssl_stream.lowest_layer().connect(*detail::solve_host(host));
		ssl_stream.handshake(ssl::stream_base::client);

		asio::streambuf request;
		std::ostream req_stream(&request);
		detail::make_request(req_stream, detail::http_method::POST, endpoint, {
			{"Host", host},
			{"User-Agent", "Brace Beta"},
			{"Content-Type", "application/x-www-form-urlencoded"},
			{"Content-Length", "0"},
			{"Connection", "Close"}
		}, false);

		std::vector<std::pair<std::string, std::string>> params = {
			{"oauth_consumer_key", key},
			{"oauth_nonce", detail::make_nonce().c_str()},
			{"oauth_signature_method", "HMAC-SHA1"},
			{"oauth_timestamp", boost::lexical_cast<std::string>(std::time(nullptr))},
			{"oauth_version", "1.0"},
		};

		std::string signature = detail::make_signature(detail::http_method::POST, detail::make_url(host, endpoint), params, secret, "");
		params.push_back({"oauth_signature", std::move(signature)});
		detail::add_request_header(req_stream, {
			{"Authorization", detail::make_authorization_header(params).c_str()}
		}, true);

		std::cout << asio::buffer_cast<const char*>(request.data());

		asio::write(ssl_stream, request);

		asio::streambuf response;
		boost::system::error_code error;

		asio::read_until(ssl_stream, response, "\r\n\r\n");
		while(asio::read(ssl_stream, response, asio::transfer_at_least(1), error));

		std::istream stream(&response);
		std::string a;
		while(std::getline(stream, a))std::cout << a << std::endl;

		return {};
#endif
	}
};
