#pragma once

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/lexical_cast.hpp>

#include <boost/optional/optional.hpp>

#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <string>
#include <ostream>
#include <utility>
#include <locale>
#include <initializer_list>

#include "http_method.hpp"

namespace brace{
	namespace detail{

		inline std::ostream &add_request_line(std::ostream &st, http_method method, const char *endpoint)
		{
			st << to_string(method) << ' ' << endpoint << " HTTP/1.1\r\n";
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

		inline std::ostream &make_request(std::ostream &st, http_method method, const char *endpoint, std::initializer_list<std::pair<const char*, const char*>> header, bool header_end = false)
		{
			add_request_line(st, method, endpoint);
			add_request_header(st, header, header_end);
			return st;
		}

		inline void encode_string(std::string &utf8)
		{
			static const char *table = "0123456789ABCDEF";
			for(auto it = utf8.begin(); it != utf8.end(); ++it){
				if(!std::isalnum(*it) && *it != '-' && *it != '.' && *it != '_' && *it != '~'){
					char hex[2];
					hex[0] = table[*it >> 4];
					hex[1] = table[*it & 0xf];
					*it++ = '%';
					it = utf8.insert(it, std::begin(hex), std::end(hex)) + 1;
				}
			}
		}

		inline std::string make_nonce()
		{
			return boost::lexical_cast<std::string>(boost::uuids::random_generator()());
		}

		// returns encoded hash
		inline std::string hmac_sha1(const std::string &key, const std::string &message)
		{
			unsigned char hash[SHA_DIGEST_LENGTH];
			unsigned int length;

			HMAC(EVP_sha1(), key.c_str(), key.length(), reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), hash, &length);

			BIO *encoder = BIO_new(BIO_f_base64());
			BIO *mem = BIO_new(BIO_s_mem());

			encoder = BIO_push(encoder, mem);
			BIO_write(encoder, hash, length);
			BIO_flush(encoder);

			BUF_MEM *buf;
			BIO_get_mem_ptr(encoder, &buf);

			std::string result(buf->data, &buf->data[buf->length - 1]);
			encode_string(result);

			BIO_free_all(encoder);

			return result;
		}

		template <class Params>
		// params : encoded, unsorted
		inline std::string make_signature(http_method method, std::string &url, Params &params, const std::string &consumer_sec, const std::string &token_sec)
		{
			using param_type = typename Params::value_type;

			std::sort(params.begin(), params.end(), [](const param_type &p1, const param_type &p2)->bool{
				return std::get<0>(p1) < std::get<0>(p2);
			});

			std::string key = consumer_sec;
			key += '&';
			key += token_sec;

			std::string message = to_string(method);
			message += '&';

			encode_string(url);
			message += url;
			message += '&';

			for(auto &param : params){
				message += std::get<0>(param);
				message += "%3D";
				message += std::get<1>(param);
				message += "%26";
			}
			message.erase(message.end() - 3, message.end());

			return hmac_sha1(key, message);
		}

		inline std::string make_url(const char *host, const char *endpoint)
		{
			std::string result = "https://";
			result += host;
			result += endpoint;
			return result;
		}

		template <class Params>
		// params : encoded, unsorted
		inline std::string make_authorization_header(const Params &params)
		{
			std::string header = "OAuth realm=\"\"";

			for(auto &param : params){
				header += ',';
				header += std::get<0>(param);
				header += '=';
				header += std::get<1>(param);
			}

			return header;
		}

		template <std::size_t N>
		inline boost::optional<std::string> parse_url_query(const std::string &query, const char (&param)[N])
		{
			std::string::size_type begin = query.find(param);
			if(begin == std::string::npos)
				return boost::none;
			begin += N - 1;

			std::string::size_type end = query.find('&', begin);
			if(end == std::string::npos)
				end = query.length();

			return query.substr(begin, end - begin);
		}

	}
}
