#define BOOST_NETWORK_ENABLE_HTTPS
#define _SCL_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x601

#include "brace/brace.hpp"

#pragma comment(lib, "libeay32MD")
#pragma comment(lib, "ssleay32MD")

int main()
try{
	oauth_handler h("", "");
	h.get_request_token();
}catch(const brace_exception &e){

}
