#include "Server.hpp"
#include "Channel.hpp"
#include "User.hpp"

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

	for (std::map<std::string, User *>::iterator it = this->_users.begin(); it != this->_users.end(); ++it) {
		delete (it->second);
	}
	this->_users.clear();

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
	int userSocket = accept(_serverSocket, NULL, NULL);
	if (userSocket == -1) {
		std::cerr << "Connection error" << std::endl;
		return ;
	}
	
	
	std::cout << "New connection accepted" << std::endl;
	User * newUser = new User(userSocket);
	this->addUser(newUser);
	this->_fds.push_back(newUser->getPollFd());

	//std::string RPL_PONG =		"PONG :pong";
	std::string RPL_WELCOME =	":ircserv 001 " + newUser->getNickName() + " :Welcome to the Internet Relay Network " + newUser->getNickName() + "\r\n";
	std::string RPL_YOURHOST =	":ircserv 002 " + newUser->getNickName() + " :Your host is " + _hostname + " , running version 42 \r\n";
	std::string RPL_CREATED = 	":ircserv 003 " + newUser->getNickName() + " :This server was created 30/10/2023\r\n";
	std::string RPL_MYINFO = 	":ircserv 004 " + newUser->getNickName() + " ircsev 42 +o +b+l+i+k+t\r\n";
	std::string RPL_ISUPPORT =	":ircserv 005 " + newUser->getNickName() + " operator ban limit invite key topic :are supported by this server\r\n";
	std::string RPL_MOTD =		":ircserv 372 " + newUser->getNickName() + " : Welcome to the ircserv\r\n";
	std::string RPL_ENDOFMOTD =	":ircserv 376 " + newUser->getNickName() + " :End of MOTD command\r\n";

	int fd = newUser->getPollFd().fd;
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);
	send(fd, RPL_CREATED.c_str(), RPL_CREATED.length(), MSG);
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);
	send(fd, RPL_ISUPPORT.c_str(), RPL_ISUPPORT.length(), MSG);
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
	//send(fd, RPL_PONG.c_str(), RPL_PONG.length(), MSG);
}

void Server::dealMessage(int userFd)
{
    char				buffer[1024];
    static std::string	msg;

    ssize_t bytesRead = recv(userFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving data" << std::endl;
    }
    else if (bytesRead == 0) {
        std::cout << "Connection closed by user" << std::endl;
        close(userFd);

		std::vector<pollfd>::iterator it = this->_fds.begin();
		while (it != this->_fds.end() && it->fd != userFd) {
			++it;
		}
		if (it != this->_fds.end()) {
			this->_fds.erase(it);
		}

		//remove user from users map?
		this->removeUser(userFd);
		
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

int	Server::nUsers(void) const
{
	return (this->_users.size());
}

int	Server::nChannels(void) const
{
	return (this->_channels.size());
}

User *	Server::getUserByName(const std::string & userName) const
{
	if (this->_users.find(userName) != this->_users.end()) {
		return (this->_users.find(userName)->second);
	}
	else {
		return (NULL);
	}
}

User *	Server::getUserByFd(int userFd) const
{
	std::map<std::string, User *>::const_iterator it = this->_users.begin();
	while (it != this->_users.end() && it->second->getPollFd().fd != userFd) {
		++it;
	}
	if (it != this->_users.end()) {
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

//users
void	Server::createUser(int fd, const std::string & userName)
{
	if (this->_channels.find(userName) == this->_channels.end()) {
		User * newUser = new User(fd);
		this->addUser(newUser);
	}
	else {
		std::cerr << "Error: cannot create user: " << userName << ", user already exists." << std::endl;
	}
}

void	Server::addUser(User * user)
{
	if (user && this->_users.find(user->getUserName()) == this->_users.end()) {
		this->_users.insert(std::make_pair(user->getUserName(), user));
	}
}

void	Server::removeUser(const std::string & userName)
{
	if (this->_users.find(userName) != this->_users.end()) {
		delete (this->_users[userName]);
		this->_users.erase(userName);
	}
}

void	Server::removeUser(int userFd)
{
	std::map<std::string, User *>::iterator it = this->_users.begin();
	while (it != this->_users.end() && it->second->getPollFd().fd != userFd) {
		++it;
	}
	if (it != this->_users.end()) {
		delete (it->second);
		this->_users.erase(it);
	}
}

/* 
void Server::sendWelcomeBackToUser(int fd) {
	std::string RPL_WELCOME =  ":ircserv 001 " + _users[fd].Nickname() + " :Welcome to the Internet Relay Network " + _users[fd].Nickname() + "\r\n";
	std::string RPL_YOURHOST = ":ircserv 002 " + _users[fd].Nickname() + " :Your host is " + hostname + " , running version 42 \r\n";
	std::string RPL_CREATED =  ":ircserv 003 " + _users[fd].Nickname() + " :This server was created 30/10/2023\r\n";
	std::string RPL_MYINFO = ":ircserv 004 " + _users[fd].Nickname() + " ircsev 42 +o +b+l+i+k+t\r\n";
	std::string RPL_ISUPPORT = ":ircserv 005 " + _users[fd].Nickname() + " operator ban limit invite key topic :are supported by this server\r\n";
	std::string RPL_MOTD = ":ircserv 372 " + _users[fd].Nickname() + " : Welcome to the ircserv\r\n";
	std::string RPL_ENDOFMOTD = ":ircserv 376 " + _users[fd].Nickname() + " :End of MOTD command\r\n";
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);
	send(fd, RPL_CREATED.c_str(), RPL_CREATED.length(), MSG);
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);
	send(fd, RPL_ISUPPORT.c_str(), RPL_ISUPPORT.length(), MSG);
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
} */