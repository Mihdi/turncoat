#ifndef UTIL_HXX
#define UTIL_HXX

#include <iostream>
#include <cstdlib>

#ifndef LOG_ERR
#define LOG_ERR(ERR) std::cerr << ERR << std::endl;
#endif // LOG_ERR

void panic(std::string err_msg)
{
	LOG_ERR(err_msg);
	std::exit(EXIT_FAILURE);
}

#endif // UTIL_HXX