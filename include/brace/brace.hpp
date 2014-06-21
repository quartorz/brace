#pragma once

#include <stdexcept>

#include <string>

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

	brace_exception(const std::string &m):message(m){}
	brace_exception(std::string &&m):message(m){}

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

#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <istream>

#include <iostream>
#include <vector>
#include <utility>
#include <initializer_list>

#include <boost/network.hpp>

boost::asio::ip::tcp::resolver::iterator solve_ip_address(const std::string &address)
{
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(address, "https");

	return resolver.resolve(query);
}

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

inline std::ostream &add_request_line(std::ostream &st, bool is_get, const char *uri)
{
	st << (is_get ? "GET " : "POST ") << uri << " HTTP/1.1\r\n";
	return st;
}

inline std::ostream &add_request_header(std::ostream &st, std::initializer_list<std::pair<const char*, const char*>> header, bool header_end = false)
{
	for(auto &t : header){
		st << std::get<0>(t) << ": " << std::get<1>(t) << "\r\n";
	}
	if(header_end)
		st << "\r\n";
	return st;
}

inline std::ostream &make_request(std::ostream &st, bool is_get, const char *uri, std::initializer_list<std::pair<const char*, const char*>> header, bool header_end = false)
{
	add_request_line(st, is_get, uri);
	add_request_header(st, header, header_end);
	return st;
}

class oauth_handler{
	std::string key, secret;
	boost::asio::ip::tcp::resolver::iterator ip;

public:
	oauth_handler(const std::string &key, const std::string &secret)
		: key(key)
		, secret(secret)
		, ip(solve_ip_address("api.twitter.com"))
	{
	}

public:
	std::string get_request_token()
	{
		namespace asio = boost::asio;
		namespace ssl = asio::ssl;
		using tcp = asio::ip::tcp;

		asio::io_service service;

		ssl::context context(service, ssl::context::sslv3_client);
		ssl::stream<tcp::socket> ssl_stream(service, context);
		ssl_stream.lowest_layer().connect(*ip);
		ssl_stream.handshake(ssl::stream_base::client);

		asio::streambuf request;
		std::ostream req_stream(&request);
		make_request(req_stream, false, "/oauth/request_token",{
				{"Host", "api.twitter.com"},
				{"User-Agent", "Brace Beta"},
				{"Content-Type", "text/plain"},
				{"Content-Length", "0"},
				{"Connection", "Close"}
		}, true);
		asio::write(ssl_stream, request);

		asio::streambuf response;
		boost::system::error_code error;

		asio::read_until(ssl_stream, response, "\r\n\r\n");
		while(asio::read(ssl_stream, response, asio::transfer_at_least(1), error));

		std::istream stream(&response);
		std::string a;
		while(std::getline(stream, a))std::cout << a << std::endl;

		return a;

//		std::cout << solve_ip_address("api.twitter.com")->endpoint().address().to_string();
		return "";
	}
};
