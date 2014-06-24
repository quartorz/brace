#pragma once

#if !defined(BOOST_NETWORK_ENABLE_HTTPS)
#define BOOST_NETWORK_ENABLE_HTTPS
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/range/iterator_range_io.hpp>

#include <boost/lexical_cast.hpp>

#include <string>
#include <tuple>
#include <ostream>
#include <vector>
#include <utility>
#include <ctime>
#include <initializer_list>

#include "http_method.hpp"
#include "functions.hpp"

namespace brace{
	namespace detail{

		class ssl_handler{
			const char *host, *endpoint;
			boost::asio::ip::tcp::endpoint ip;
			http_method method;

		public:
			ssl_handler(const char *host, const char *endpoint, http_method method)
				: host(host)
				, endpoint(endpoint)
				, method(method)
			{
			}

			bool resolve_host()
			{
				boost::asio::io_service io_service;
				boost::asio::ip::tcp::resolver resolver(io_service);
				boost::asio::ip::tcp::resolver::query query(host, "https");
				auto it = resolver.resolve(query);

				if(it != boost::asio::ip::tcp::resolver::iterator()){
					ip = *it;
					return true;
				} else
					return false;
			}

			// returns HTTP status code and content
			std::tuple<int, std::string> connect(
				const std::pair<std::string, std::string> &consumer,
				const std::pair<std::string, std::string> &token,
				std::initializer_list<std::pair<std::string, std::string>> ext_oauth_params ={},
				std::initializer_list<std::pair<std::string, std::string>> ext_http_query ={})
			{
				namespace asio = boost::asio;
				namespace ssl = asio::ssl;
				using tcp = asio::ip::tcp;

				try{
					asio::io_service service;

					ssl::context context(service, ssl::context::sslv3_client);
					ssl::stream<tcp::socket> ssl_stream(service, context);
					ssl_stream.lowest_layer().connect(ip);
					ssl_stream.handshake(ssl::stream_base::client);

					asio::streambuf request;
					std::ostream req_stream(&request);
					make_request(req_stream, method, endpoint, {
						{"Host", host},
						{"User-Agent", "Brace Beta"},
						{"Content-Type", "application/x-www-form-urlencoded"},
						{"Connection", "Close"},
					}, false);

					std::vector<std::pair<std::string, std::string>> oauth_params ={
						{"oauth_consumer_key", std::get<0>(consumer)},
						{"oauth_nonce", detail::make_nonce()},
						{"oauth_signature_method", "HMAC-SHA1"},
						{"oauth_timestamp", boost::lexical_cast<std::string>(std::time(nullptr))},
						{"oauth_version", "1.0"},
					};

					if(std::get<0>(token).length() != 0)
						oauth_params.emplace_back("oauth_token", std::get<0>(token));

					oauth_params.insert(oauth_params.end(), ext_oauth_params);

					std::string signature = make_signature(method, make_url(host, endpoint), oauth_params, std::get<1>(consumer), std::get<1>(token));
					oauth_params.emplace_back("oauth_signature", std::move(signature));
					add_request_header(req_stream, {
						{"Authorization", make_authorization_header(oauth_params).c_str()},
					}, true);

					asio::write(ssl_stream, request);

					int status;
					{
						asio::streambuf response;
						asio::read_until(ssl_stream, response, "\r\n\r\n");
						auto buf = asio::buffer_cast<const char*>(response.data());

						status = boost::lexical_cast<int>(boost::make_iterator_range(buf + 9, buf + 12));
					}

					std::string content;
					{
						asio::streambuf response;
						boost::system::error_code error;
						while(asio::read(ssl_stream, response, asio::transfer_at_least(1), error));
						content.assign(asio::buffer_cast<const char*>(response.data()), response.size());
					}

					return std::make_tuple(status, content);
				} catch(boost::system::system_error &e){
					throw e;
				}
			}
		};

	}
}
