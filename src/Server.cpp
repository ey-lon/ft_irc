#include "Server.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "Utils.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <netdb.h>

#define MSG (MSG_DONTWAIT | MSG_NOSIGNAL)

//--------------------------------------------------
//constructors, destructors, ...
Server::Server(void) : _port(6667) {}

Server::Server(int port, const std::string & password) {
	//if (...) { //port error check
		this->_port = port;
	//}
	/* else {
		throw (std::exception());
	} */

	//if (...) { //password error check
		this->_password = password;
	//}
	/* else {
		throw (std::exception());
	} */
}

Server::~Server(void) {
	close(this->_serverSocket);
	this->_fds.clear();
	for (std::map<std::string, Channel *>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
		delete (it->second);
	}
	this->_channels.clear();
	for (std::map<int, User *>::iterator it = this->_users.begin(); it != this->_users.end(); ++it) {
		delete (it->second);
	}
	this->_users.clear();
}

//--------------------------------------------------
void	Server::user(std::vector<std::string> argv, User * user) {
	//USER <username>
	if (argv.size() < 2) {
		; //error
	}
	else if (getUserByUserName(argv[1])) {
		; //error <-- user already exists
	}
	else {
		user->setUserName(argv[1]);
	}
}

void	Server::nick(std::vector<std::string> argv, User * user) {
	//NICK <nickname>
	if (argv.size() < 2) {
		; //error
	}
	else {
		user->setNickName(argv[1]);
	}
}

void	Server::privmsg(std::vector<std::string> argv, User * user) {
	//PRIVMSG <user/channel> <message>
	if (argv.size() < 3) {
		//error <-- not enough arguments
		return ;
	}
	bool		isDestChannel = false;
	std::string	destName = argv[1];
	if (destName[0] == '#') {
		isDestChannel = true;
		destName = destName.erase(0, 1);
	}
	std::string msgText = argv[2];
	if (!msgText.empty()) {
		if (isDestChannel) {
			Channel * channelDest = this->getChannelByName(destName);
			if (channelDest) {
				//send message to all user in channel
			}
			else {
				//channel not found
			}
		}
		else {
			User * userDest = this->getUserByUserName(destName);
			if (userDest) {
				//send message to user
			}
			else {
				//user not found
			}
		}
	}
}

void	Server::join(std::vector<std::string> argv, User * user) {
	//JOIN <ch1,ch2,...,chn> <key1,key2,...,keyn>
	if (argv.size() < 2) {
		; //error
		return;
	}
	std::vector<std::string> channelVec = splitString(argv[1], ',');	// <-- split into channel vector
	std::vector<std::string> keyVec;
	if (argv.size() > 2) {
		keyVec = splitString(argv[2], ','); 							// <-- split into password vector
	}
	for (size_t i = 0; i < channelVec.size(); ++i) {
		Channel * channel = getChannelByName(channelVec[i]);
		if (channel) {
			if (channel->isInviteOnly()) {
				; //error <-- cannot join
			}
			else if (!channel->getPassword().empty() && i >= keyVec.size()) {
				; //error <-- user didn't send password
			}
			else if (!channel->getPassword().empty() && (channel->getPassword() != keyVec[i])) {
				; //error <-- password doesn't match
			}
			else {
				; //password not needed or correct
			}
		}
		else {
			channel = createChannel(channelVec[i]);
			if (i < keyVec.size() && !keyVec[i].empty()) {
				channel->setPassword(keyVec[i]);
			}
			//send message to user
		}
	}
	/*
	//reply
	std::string rpl;
	rpl = ":ircserv 332 " + user->getNickName() + " #" + channel->getName() + " :Channel Topic\n";
	send(user->getFd(), rpl.c_str(), rpl.length(), MSG);
	rpl = ":ircserv 333 " + user->getNickName() + " #" + channel->getName() + " operator 1234567890\n";
	send(user->getFd(), rpl.c_str(), rpl.length(), MSG);
	rpl = ":prefix JOIN " + channel->getName() + "\n";
	send(user->getFd(), rpl.c_str(), rpl.length(), MSG);
	*/
}

void	Server::kick(std::vector<std::string> argv, User * user) {
	//KICK <#channel> <user> <message>
	if (argv.size() < 3) {
		//error
		return ;
	}
	if (argv[1][0] != '#') {
		; //error
		return ;
	}
	std::string channelName = argv[1].substr(1);
	std::string userName = argv[2];
	Channel * channel = getChannelByName(channelName);
	if (channel) {
		if (!channel->isUserOperator(user->getUserName())) {
			; //error <-- user is not operator
		}
		else {
			channel->removeUser(userName);
			if (argv.size() > 3 && !argv[3].empty()) {
				; //send message?
			}
		}
	}
}

void	Server::pass(std::vector<std::string> argv, User * user) {
	//PASS <password>
	if (argv.size() < 2) {
		; //error
		return;
	}
	if (argv[1] != this->_password) {
		;//error
		return ;
	}
	user->authorize();
	//send message to client?
}

void	Server::welcome(User * user) {
	std::string nickname = user->getNickName();
	std::string RPL_WELCOME =	":ircserv 001 " + nickname + " :Welcome to the Internet Relay Network " + nickname + "\r\n";
	std::string RPL_YOURHOST =	":ircserv 002 " + nickname + " :Your host is " + _hostname + " , running version 42 \r\n";
	std::string RPL_CREATED = 	":ircserv 003 " + nickname + " :This server was created 30/10/2023\r\n";
	std::string RPL_MYINFO = 	":ircserv 004 " + nickname + " ircsev 42 +o +b+l+i+k+t\r\n";
	std::string RPL_ISUPPORT =	":ircserv 005 " + nickname + " operator ban limit invite key topic :are supported by this server\r\n";
	std::string RPL_MOTD =		":ircserv 372 " + nickname + " : Welcome to the ircserv\r\n";
	std::string RPL_ENDOFMOTD =	":ircserv 376 " + nickname + " :End of MOTD command\r\n";

	int fd = user->getFd();
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);
	send(fd, RPL_CREATED.c_str(), RPL_CREATED.length(), MSG);
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);
	send(fd, RPL_ISUPPORT.c_str(), RPL_ISUPPORT.length(), MSG);
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
}

void	Server::authorization(std::vector<std::string> argv, User *user) {
	//here I can expect either CAP LS 302 or PASS command
	if (argv.empty()) {
		return;
	}
	else if (argv[0] == "CAP") {
		;
	}
	else if (argv[0] == "PASS") {
		this->pass(argv, user);
	}
}

void	Server::login(std::vector<std::string> argv, User *user) {
	if (argv.empty()) {
		return;
	}
	else if (argv[0] == "NICK") {
		this->nick(argv, user);
	}
	else if (argv[0] == "USER") {
		this->user(argv, user);
	}
	if (!user->getNickName().empty() && !user->getUserName().empty()) {
		user->authenticate();
		this->welcome(user);
	}
}

void	Server::dealCommand(std::vector<std::string> argv, User *user) {
	if (argv.empty()) {
		return;
	}
	//commands
	else if (argv[0] == "CAP") {
		;
	}
	else if (argv[0] == "NICK") {
		this->nick(argv, user);
	}
	else if (argv[0] == "JOIN") {
		this->join(argv, user);
	}
	else if (argv[0] == "PRIVMSG") {
		this->privmsg(argv, user);
	}
	//operators only
	else if (argv[0] == "KICK") {
		this->kick(argv, user);
	}
	else if (argv[0] == "INVITE") {
		this->invite(argv, user);
	}
	else if (argv[0] == "TOPIC") {
		this->topic(argv, user);
	}
	else if (argv[0] == "MODE") {
		this->mode(argv, user);
	}
	else {
		//unknown command <-- send to client?
		;
	}
}

int Server::dealMessage(int userFd) {
	User * ptr = this->getUserByFd(userFd);
	if (!ptr) {												// <-- don't know how it could happen, but you never know.
		std::cerr << "Error: User not found" << std::endl;
		return (2);
	}

    char				buffer[1024];
    ssize_t bytesRead = recv(userFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        std::cout << "Connection with user_fd [" << ptr->getFd() << "] terminated." << std::endl;
		ptr->setMessage("");
        close(userFd);
		this->removeUser(userFd);
		return (1);
    }
    else {
		buffer[bytesRead] = '\0'; // <--- NECESSARY
		ptr->setMessage(ptr->getMessage() + buffer);
		if (ptr->getMessage().find('\n') != std::string::npos) {
			std::vector<std::string> argv = parseInput(ptr->getMessage());
			if (!ptr->isAuthorized()) {
				this->authorization(argv, ptr);
			}
			else if (!ptr->isAuthenticated()) {
				this->login(argv, ptr);
			}
			else {
				this->dealCommand(argv, ptr);
			}
			std::cout << "[" << ptr->getFd() << "]: " << ptr->getMessage() << std::endl;
			ptr->setMessage("");
		}
		return (0);
    }
}

void	Server::newConnection(void) {
	int userSocket = accept(_serverSocket, NULL, NULL);
	if (userSocket == -1) {
		std::cerr << "Connection error" << std::endl;
		return ;
	}
	std::cout << "New connection accepted, user_fd [" << userSocket << "]" << std::endl;
	this->createUser(userSocket);
}

void	Server::loop(void) {
	std::cout << "Server with ip: " << _ip << ", listening on port: " << _port << std::endl;

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
                else if (this->dealMessage(this->_fds[i].fd) == 1) {
					--i; // <-- if client disconnects, its pollfd gets removed from the vector, so the next pollfd is at the index of the one that got removed, right? 
				}
            }
        }
    }
}

void	Server::start(void) {
	// Create a socket
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1) {
		throw ("socket creation");
    }
	// Set socket options
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        throw ("socket option settings");
    }
    // Set up server address structure
	_serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = INADDR_ANY;	// Listen on all available interfaces
    _serverAddress.sin_port = htons(this->_port);
	// Bind the socket
    if (bind(_serverSocket, (struct sockaddr*)&_serverAddress, sizeof(_serverAddress)) == -1) {
        close(_serverSocket);
        throw ("socket binding");
    }
    // Listen for incoming connections
    if (listen(_serverSocket, SOMAXCONN) == -1) {
        close(_serverSocket);
        throw ("socket listening");
    }
	// Get hostname
	if (gethostname(_hostname, sizeof(_hostname)) == -1) {
		close(_serverSocket);
		throw ("gethostname");
	}
	// Get ip
	struct hostent *host_entry = gethostbyname(_hostname);
	if (host_entry == NULL) {
		close(_serverSocket);
		throw ("gethostbyname");
	}
	_ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
}

//--------------------------------------------------
//getters
const std::string &	Server::getPassword(void) const {
	return (this->_password);
}

int	Server::getPort(void) const {
	return (this->_port);
}

int	Server::nUsers(void) const {
	return (this->_users.size());
}

int	Server::nChannels(void) const {
	return (this->_channels.size());
}

//--------------------------------------------------
//setters
void	Server::setPassword(const std::string &password) {
	//if (...) { //password error check
		this->_password = password;
	//}
}

//--------------------------------------------------
//channels

Channel *	Server::getChannelByName(const std::string & channelName) const {
	if (this->_channels.find(channelName) != this->_channels.end()) {
		return (this->_channels.find(channelName)->second);
	}
	else {
		return (NULL);
	}
}
Channel *	Server::createChannel(const std::string & channelName) {
	if (this->_channels.find(channelName) == this->_channels.end()) {
		Channel * newChannel = new Channel(channelName);
		this->_channels.insert(std::make_pair(channelName, newChannel));	// <-- add channel to map
		return (newChannel);
	}
	else {
		return (NULL);
	}
}

void	Server::removeChannel(const std::string & channelName) {
	if (this->_channels.find(channelName) != this->_channels.end()) {
		delete (this->_channels[channelName]);								// <-- delete Channel *
		this->_channels.erase(channelName);									// <-- remove channel from map
	}
}

//--------------------------------------------------
//users
User *	Server::getUserByFd(int userFd) const {
	if (this->_users.find(userFd) != this->_users.end()) {
		return (this->_users.find(userFd)->second);
	}
	else {
		return (NULL);
	}

}

User *	Server::getUserByUserName(const std::string & userName) const {
	std::map<int, User *>::const_iterator it = this->_users.begin();
	while (it != this->_users.end() && it->second->getUserName() != userName) {
		++it;
	}
	if (it != this->_users.end()) {
		return (it->second);
	}
	else {
		return (NULL);
	}
}

User *	Server::createUser(int fd) {
	if (this->_users.find(fd) == this->_users.end()) {
		User * newUser = new User(fd);
		this->_users.insert(std::make_pair(fd, newUser));					// <-- add user to map
		this->_fds.push_back(newUser->getPollFd());							// <-- add user pollfd to vector
		return (newUser);
	}
	else {
		return (NULL);
	}
}

void	Server::removeUser(int userFd) {
	if (this->_users.find(userFd) != this->_users.end()) {

 		//remove user from channels
		std::string userName = this->getUserByFd(userFd)->getUserName();
		for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++) {
			if (it->second->isUserPresent(userName)) {	
				it->second->removeUser(userName);
			}
		}

		delete (this->_users[userFd]);										// <-- delete User *
		this->_users.erase(userFd);											// <-- remove user from map

		std::vector<pollfd>::iterator it = this->_fds.begin();
		while (it != this->_fds.end() && it->fd != userFd) {
			++it;
		}
		if (it != this->_fds.end()) {
			this->_fds.erase(it);											// <-- remove user pollfd from vector
		}
	}
}

//--------------------------------------------------
//UNUSED
/*
void	set username(User * user, const std::string & userName) {
	if (this->_channels.find(userName) == this->_channels.end()) {
		user->setUserName(userName);
	}
	else {
		std::cerr << "Error: cannot set username to: " << userName << ", user already exists." << std::endl;
	}
}
*/

/*
void	set channelname (Channel * channel, const std::string & channelName) {
	if (this->_channels.find(channelName) == this->_channels.end()) {
		channel->setName(channelName);
	}
	else {
		std::cerr << "Error: cannot set channel name to: " << channelName << ", channel already exists." << std::endl;
	}
}
*/

//temporary user name -------------------------------
/*
int length = snprintf(NULL, 0, "%d", userSocket) + 1;
char * client_n_str = new char[length];
sprintf(client_n_str, "%d", userSocket);
std::string client_name = "user fd.";
client_name.append(client_n_str);
delete [] (client_n_str);
*/
//---------------------------------------------------


//std::string RPL_PONG =		"PONG :pong";
//send(fd, RPL_PONG.c_str(), RPL_PONG.length(), MSG);


//CAP :  c==3
//PASS : c==3
//NICK : c==3
//USER : c==3