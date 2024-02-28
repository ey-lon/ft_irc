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
		//Server server(av[1], av[2]);
		Server server;

		server.start();
		server.loop();
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return (0);
}