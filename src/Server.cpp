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
}

void	Server::newConnection(void)
{
	int clientSocket = accept(_serverSocket, nullptr, nullptr);
	if (clientSocket == -1) {
		std::cerr << "Connection error" << std::endl;
		return ;
	}
	
	std::cout << "New connection accepted" << std::endl;
	Client * newClient = new Client(clientSocket);
	this->addClient(newClient);

	//prompt?
}

/* void Server::dealMessage(int clientFd)
{
    char			buffer[1024];
    std::string		msg;

    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving data" << std::endl;
    }
    else if (bytesRead == 0) {
        std::cout << "Connection closed by client" << std::endl;
        close(clientFd);
    }
    else {
        //std::cout << "Received " << bytesRead << " bytes: " << buffer << std::endl;
		msg += buffer;
		if (msg.find('\n') != std::string::npos) {
			std::cout << msg << std::endl;
			if (msg == "exit\n") {
				close(_serverSocket);
				throw (std::exception());
			}
			msg.clear();
    }
} */

void	Server::loop(void)
{
	std::vector<pollfd> fds;
    fds.push_back({_serverSocket, POLLIN, 0});
    

	pollfd	serverPollFd;
	serverPollFd.fd = _serverSocket;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	

	std::string msg;
    while (true) {
        //int numready = poll(fds.data(), fds.size(), -1);
		int numready = poll(&serverPollFd, nClients() + 1, -1);
		for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->second->getRevents() & POLLIN) {
                if (it->second->getFd() == _serverSocket) {
                    this->newConnection();
                }
                else {
					this->dealMessage();
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
