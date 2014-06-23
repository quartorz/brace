#define BOOST_NETWORK_ENABLE_HTTPS
#define _SCL_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x601

#include "brace/brace.hpp"

#pragma comment(lib, "libeay32MD")
#pragma comment(lib, "ssleay32MD")

int main()
{
	oauth_handler h("jpQ5IemAsibklW53CxDLag", "zR5FloubeTVnOQjs0Nbk7iHVYtgaGqtQWuv1AFyZg");
	h.get_request_token();
}
