#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>

Server::Server(void)
{
	this->_port = 6667;
}

Server::~Server(void)
{
	close(this->_serverSocket);

	for (std::map<std::string, Channel *>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
		delete (it->second);
	}
	this->_channels.clear();

	for (std::map<std::string, Client *>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		delete (it->second);
	}
	this->_clients.clear();

	this->_fds.clear();
}

Server::Server(int port, const std::string & password)
{
	//if (...) { //controlli sulla porta
		this->_port = port;
	//}
	/* else {
		throw (std::exception());
	} */

	//if (...) { //controlli sulla password
		this->_password = password;
	//}
	/* else {
		throw (std::exception());
	} */

	//aprire server
}

//

void	Server::start(void)
{
	// Create a socket
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        throw (std::exception());
    }

	// Set socket options
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Error setting socket options" << std::endl;
        throw (std::exception());
    }

    // Set up server address structure
	_serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    _serverAddress.sin_port = htons(this->_port); // Example port number, you can choose any available port

	// Bind the socket
    if (bind(_serverSocket, (struct sockaddr*)&_serverAddress, sizeof(_serverAddress)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(_serverSocket);
        throw (std::exception());
    }

    // Listen for incoming connections
    if (listen(_serverSocket, SOMAXCONN) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(_serverSocket);
        throw (std::exception());
    }

	if (gethostname(_hostname, sizeof(_hostname)) == -1) {
		throw std::runtime_error ("ERROR: gethostname failed");
	}
}

void	Server::newConnection(void)
{
	int clientSocket = accept(_serverSocket, NULL, NULL);
	if (clientSocket == -1) {
		std::cerr << "Connection error" << std::endl;
		return ;
	}
	
	std::cout << "New connection accepted" << std::endl;
	Client * newClient = new Client(clientSocket);
	this->addClient(newClient);
	this->_fds.push_back(newClient->getPollFd());

	std::string RPL_WELCOME =	":ircserv 001 " + newClient->getNickName() + " :Welcome to the Internet Relay Network " + newClient->getNickName() + "\r\n";
	std::string RPL_YOURHOST =	":ircserv 002 " + newClient->getNickName() + " :Your host is " + _hostname + " , running version 42 \r\n";
	std::string RPL_CREATED = 	":ircserv 003 " + newClient->getNickName() + " :This server was created 30/10/2023\r\n";
	std::string RPL_MYINFO = 	":ircserv 004 " + newClient->getNickName() + " ircsev 42 +o +b+l+i+k+t\r\n";
	std::string RPL_ISUPPORT =	":ircserv 005 " + newClient->getNickName() + " operator ban limit invite key topic :are supported by this server\r\n";
	std::string RPL_MOTD =		":ircserv 372 " + newClient->getNickName() + " : Welcome to the ircserv\r\n";
	std::string RPL_ENDOFMOTD =	":ircserv 376 " + newClient->getNickName() + " :End of MOTD command\r\n";

	int fd = newClient->getPollFd().fd;
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);
	send(fd, RPL_CREATED.c_str(), RPL_CREATED.length(), MSG);
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);
	send(fd, RPL_ISUPPORT.c_str(), RPL_ISUPPORT.length(), MSG);
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
}

void Server::dealMessage(int clientFd)
{
    char				buffer[1024];
    static std::string	msg;

    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving data" << std::endl;
    }
    else if (bytesRead == 0) {
        std::cout << "Connection closed by client" << std::endl;
        close(clientFd);

		std::vector<pollfd>::iterator it = this->_fds.begin();
		while (it != this->_fds.end() && it->fd != clientFd) {
			++it;
		}
		if (it != this->_fds.end()) {
			this->_fds.erase(it);
		}

		//remove client from clients map?
		this->removeClient(clientFd);
		
		msg.clear();
    }
    else {
		buffer[bytesRead] = '\0'; // <--- NECESSARY
		msg += buffer;
		if (msg.find('\n') != std::string::npos) {
			std::cout << msg << std::endl;
			msg.clear();
		}
    }
}

void	Server::loop(void)
{
	std::cout << "Server listening on port " << _port << "..." << std::endl;

	pollfd	serverPollFd;
	serverPollFd.fd = this->_serverSocket;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;

    this->_fds.push_back(serverPollFd);

    while (true) {
		poll(this->_fds.data(), this->_fds.size(), -1);
		for (size_t i = 0; i < this->_fds.size(); ++i) {
            if (this->_fds[i].revents & POLLIN) {
                if (this->_fds[i].fd == this->_serverSocket) {
                    this->newConnection();
                }
                else {
					this->dealMessage(this->_fds[i].fd);
                }
            }
        }
    }
}

//getters
const std::string &	Server::getPassword(void) const
{
	return (this->_password);
}

int	Server::getPort(void) const
{
	return (this->_port);
}

int	Server::nClients(void) const
{
	return (this->_clients.size());
}

int	Server::nChannels(void) const
{
	return (this->_channels.size());
}

Client *	Server::getClientByName(const std::string & clientName) const
{
	if (this->_clients.find(clientName) != this->_clients.end()) {
		return (this->_clients.find(clientName)->second);
	}
	else {
		return (NULL);
	}
}

Client *	Server::getClientByFd(int clientFd) const
{
	std::map<std::string, Client *>::const_iterator it = this->_clients.begin();
	while (it != this->_clients.end() && it->second->getPollFd().fd != clientFd) {
		++it;
	}
	if (it != this->_clients.end()) {
		return (it->second);
	}
	else {
		return (NULL);
	}
}

Channel *	Server::getChannelByName(const std::string & channelName) const
{
	if (this->_channels.find(channelName) != this->_channels.end()) {
		return (this->_channels.find(channelName)->second);
	}
	else {
		return (NULL);
	}
}

//setters
void	Server::setPassword(const std::string &password)
{
	//if (...) { //controlli sulla password
		this->_password = password;
	//}
}

//channels
void	Server::createChannel(const std::string & channelName)
{
	if (this->_channels.find(channelName) == this->_channels.end()) {
		Channel * newChannel = new Channel(channelName);
		this->addChannel(newChannel);
	}
	else {
		std::cerr << "Error: cannot create channel: #" << channelName << ", channel already exists." << std::endl;
	}
}

void	Server::addChannel(Channel * channel)
{
	if (channel && this->_channels.find(channel->getName()) == this->_channels.end()) {
		this->_channels.insert(std::make_pair(channel->getName(), channel));
	}
}

void	Server::removeChannel(const std::string & channelName)
{
	if (this->_channels.find(channelName) != this->_channels.end()) {
		delete (this->_channels[channelName]);
		this->_channels.erase(channelName);
	}
}

//clients
void	Server::createClient(int fd, const std::string & clientName)
{
	if (this->_channels.find(clientName) == this->_channels.end()) {
		Client * newClient = new Client(fd);
		this->addClient(newClient);
	}
	else {
		std::cerr << "Error: cannot create client: " << clientName << ", client already exists." << std::endl;
	}
}

void	Server::addClient(Client * client)
{
	if (client && this->_clients.find(client->getUserName()) == this->_clients.end()) {
		this->_clients.insert(std::make_pair(client->getUserName(), client));
	}
}

void	Server::removeClient(const std::string & clientName)
{
	if (this->_clients.find(clientName) != this->_clients.end()) {
		delete (this->_clients[clientName]);
		this->_clients.erase(clientName);
	}
}

void	Server::removeClient(int clientFd)
{
	std::map<std::string, Client *>::iterator it = this->_clients.begin();
	while (it != this->_clients.end() && it->second->getPollFd().fd != clientFd) {
		++it;
	}
	if (it != this->_clients.end()) {
		delete (it->second);
		this->_clients.erase(it);
	}
}

/* 
void Server::sendWelcomeBackToClient(int fd) {
	std::string RPL_WELCOME =  ":ircserv 001 " + _clients[fd].Nickname() + " :Welcome to the Internet Relay Network " + _clients[fd].Nickname() + "\r\n";
	std::string RPL_YOURHOST = ":ircserv 002 " + _clients[fd].Nickname() + " :Your host is " + hostname + " , running version 42 \r\n";
	std::string RPL_CREATED =  ":ircserv 003 " + _clients[fd].Nickname() + " :This server was created 30/10/2023\r\n";
	std::string RPL_MYINFO = ":ircserv 004 " + _clients[fd].Nickname() + " ircsev 42 +o +b+l+i+k+t\r\n";
	std::string RPL_ISUPPORT = ":ircserv 005 " + _clients[fd].Nickname() + " operator ban limit invite key topic :are supported by this server\r\n";
	std::string RPL_MOTD = ":ircserv 372 " + _clients[fd].Nickname() + " : Welcome to the ircserv\r\n";
	std::string RPL_ENDOFMOTD = ":ircserv 376 " + _clients[fd].Nickname() + " :End of MOTD command\r\n";
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);
	send(fd, RPL_CREATED.c_str(), RPL_CREATED.length(), MSG);
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);
	send(fd, RPL_ISUPPORT.c_str(), RPL_ISUPPORT.length(), MSG);
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
} */