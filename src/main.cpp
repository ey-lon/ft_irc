#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

Server * globalServerPtr = NULL;
/* 
static void	sigHandler(int signal, siginfo_t *info, void *ucontext) {
	if (info->si_pid == getpid()) {				// <-- sender is server
		if (signal == SIGINT) {					// <-- signal is SIGINT (CTRL + C)
			if (globalServerPtr != NULL) {		// <-- server has been set
				globalServerPtr->stop();		// <-- interrupt server loop
			}
		}
	}
	(void)ucontext;
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Error: invalid arguments." << std::endl;
		std::cout << "./ircserv <port> <password>" << std::endl;
		return (1);
	
	}

	struct sigaction	sa;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);
	sa.sa_flags = SA_SIGINFO | SA_NODEFER;
	sa.sa_sigaction = sigHandler;
	sigaction(SIGINT, &sa, NULL);

	try {
		Server server(av[1], av[2]);
		globalServerPtr = &server;

		server.init();
		server.run();

	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (const char * str) {
		std::cerr << "Error: " << str << std::endl;
	}
	return (0);
} */

void	sigHandler(int signal) {
	if (signal == SIGINT) {
		if (globalServerPtr != NULL) {
			globalServerPtr->stop();
		}
	}
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Error: invalid arguments." << std::endl;
		std::cout << "./ircserv <port> <password>" << std::endl;
		return (1);
	
	}
	signal(SIGINT, sigHandler);

	try {
		Server server(av[1], av[2]);
		globalServerPtr = &server;

		server.init();
		server.run();

	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (const char * str) {
		std::cerr << "Error: " << str << std::endl;
	}
	return (0);
}
