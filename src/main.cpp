#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdlib>

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Error: invalid arguments." << std::endl;
		std::cout << "./ircserv <port> <password>" << std::endl;
		return (1);
	}

	try {
		Server server(av[1], av[2]);

		server.start();
		server.loop();
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (const char * str) {
		std::cerr << "Error: " << str << std::endl;
	}
	return (0);
}
