#define BOOST_NETWORK_ENABLE_HTTPS

#if defined(_WIN32)
# define _SCL_SECURE_NO_WARNINGS
# define _WIN32_WINNT 0x601

# if defined(NDEBUG)
#  pragma comment(lib, "libeay32MD")
#  pragma comment(lib, "ssleay32MD")
# else
#  pragma comment(lib, "libeay32MDd")
#  pragma comment(lib, "ssleay32MDd")
# endif
#endif

#include "brace/oauth_handler.hpp"

int main()
{
	brace::oauth_handler h("jpQ5IemAsibklW53CxDLag", "zR5FloubeTVnOQjs0Nbk7iHVYtgaGqtQWuv1AFyZg");
	auto url = h.get_authorize_url();
	if(url){
		std::cout << *url << std::endl;

		std::string pin;
		std::cin >> pin;

		auto &response = h.set_pin(pin);
		if(response)
			std::cout << *response;
	}
}
