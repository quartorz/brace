#define BOOST_NETWORK_ENABLE_HTTPS
#define _SCL_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x601

#include "brace/brace.hpp"

#pragma comment(lib, "libeay32MD")
#pragma comment(lib, "ssleay32MD")

int main()
{
	oauth_handler h("jpQ5IemAsibklW53CxDLag", "zR5FloubeTVnOQjs0Nbk7iHVYtgaGqtQWuv1AFyZg");
	auto token = h.get_request_token();
	if(token)
		std::cout << "request token: " << std::get<0>(*token) << std::endl << "request token secret: " << std::get<1>(*token) << std::endl;
}
