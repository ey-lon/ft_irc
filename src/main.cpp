#include "Server.hpp"
#include <iostream>

int main()
{
	/* if (ac != 3)
	{
		std::cerr << "Error: invalid arguments." << std::endl;
		std::cout << "./ircserv <port> <password>" << std::endl;
		return (1);
	} */

	try {
		Server server;

		server.start();
	}
	catch (std::exception &) {
		std::cerr << "Error" << std::endl;
	}
	return (0);
}