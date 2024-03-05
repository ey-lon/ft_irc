#include "Server.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "Utils.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

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

//==================================================
//COMMANDS

//--------------------------------------------------
//authorization
void	Server::cap(std::vector<std::string> argv, User * user) {
	//CAP LS/REQ/ACK

	//ignore Client Capabilities Negotiation always?
}

void	Server::pass(std::vector<std::string> argv, User * user) {
	//PASS <password>
	if (argv.size() < 2) {
		return ;
	}
	else if (argv[1] != this->_password) {
		; //error: password doesn't match
	}
	else {
		user->authorize();
		//send message to client?
	}
}

//--------------------------------------------------
//authentication
void	Server::user(std::vector<std::string> argv, User * user) {
	//USER <username>
	if (argv.size() < 2) {
		; //error
	}
	else if (getUserByUserName(argv[1])) {
		; //error: user already exists
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
	else if (getUserByNickName(argv[1])) {
		; //error: user already exists
	}
	else {
		user->setNickName(argv[1]);
	}
}

//--------------------------------------------------
//other
void	Server::privmsg(std::vector<std::string> argv, User * user) {
	//PRIVMSG <nickname/channel> <message>
	if (argv.size() < 3) {
		return ;
	}
	std::string	destName = argv[1];
	std::string msgText = argv[2];
	if (!msgText.empty()) {
		if (destName[0] == '#') {
			destName.erase(0, 1);
			Channel * channelDest = this->getChannelByName(destName);
			if (channelDest) {
				//send message to all user in channel
			}
			else {
				//error: channel not found
			}
		}
		else {
			User * userDest = this->getUserByNickName(destName);
			if (userDest) {
				; //send message to user
			}
			else {
				; //error: user not found
			}
		}
	}
}

void	Server::join(std::vector<std::string> argv, User * user) {
	//JOIN <ch1,ch2,...,chn> [<key1,key2,...,keyn>]
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
		if (channelVec[i][0] == '#') {
			channelVec[i].erase(0, 1);
		}
		Channel * channel = getChannelByName(channelVec[i]);
		if (channel) {	// <-- channel exists
			if (channel->hasFlag(i)) {
				;//error: channel is invite only, user cannot join
			}
			else if (!channel->getPassword().empty() && i >= keyVec.size()) {
				;//error: password needed but user didn't send it
			}
			else if (!channel->getPassword().empty() && (channel->getPassword() != keyVec[i])) {
				;//error: password needed, user sent it but it doesn't match
			}
			else if (channel->hasFlag('l') && channel->getUsersLimit() >= channel->nUsers()) {
				;//error: too many users
			}
			else { //user can join because password is not needed or matches
				channel->addUser(user);
				//send REPLY messages to users
			}
		}
		else {			// <-- channel doesn't exist
			channel = createChannel(channelVec[i]);
			if (i < keyVec.size() && !keyVec[i].empty()) {
				channel->setPassword(keyVec[i]);
			}
			channel->addUser(user);
			channel->promoteUser(user->getNickName());
			//send REPLY messages to users
		}
	}
}

//--------------------------------------------------
//operators only
void	Server::kick(std::vector<std::string> argv, User * user) {
	//KICK <#channel> <nickname> [<message>]
	if (argv.size() < 3) {
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	std::string nickName = argv[2];
	Channel * channel = getChannelByName(channelName);
	if (channel) {
		if (!channel->isUserOperator(user->getNickName())) {
			; //error: user is not operator
		}
		else {
			channel->removeUser(nickName);
			if (argv.size() > 3 && !argv[3].empty()) {
				; //send optional message
			}
		}
	}
}

void	Server::invite(std::vector<std::string> argv, User * user) {
	//INVITE <nickname> <channel>
	if (argv.size() < 3) {
		return ;
	}
	std::string channelName = argv[2];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	std::string nickName = argv[1];
	Channel * channel = this->getChannelByName(channelName);
	if (channel) {
		if (!channel->isUserOperator(user->getNickName())) {
			; //error: user is not operator
		}
		else {
			User * userInv = getUserByNickName(nickName);
			if (userInv) {
				channel->addUser(userInv);
			}
			else {
				; //error: user to invite not found
			}			
		}
	}
	else {
		; //error: channel not found
	}
}

void	Server::topic(std::vector<std::string> argv, User * user) {
	//TOPIC <channel> [<topic>]
	if (argv.size() < 2) {
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	Channel * channel = this->getChannelByName(channelName);
	if (channel) {
		if (channel->hasFlag('t') && !channel->isUserOperator(user->getNickName())) {
			; //error: user is not operator
		}
		else if (argv.size() > 2) {
			channel->setTopic(argv[2]);
		}
		else {
			; //view topic
		}
	}
	else {
		; //error: channel not found
	}
}

void	Server::mode(std::vector<std::string> argv, User * user) {
	//MODE <channel> {[+|-]|i|t|k|o|l} [<limit>] [<user>]
	if (argv.size() < 3) { //user didn't send channel or flags
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	Channel * channel = this->getChannelByName(channelName);
	if (channel) {
		if (!channel->isUserOperator(user->getNickName())) {
			; //error: user is not operator
		}
		else {
			std::string flags = argv[2];
			int ac = 0;
			if (flags[0] == '+') {
				for (size_t i = 1; i < flags.length(); i++) {
					if (flags[i] == 'o') {
						if (argv.size() > 3 + ac) {
							User * user = channel->getUserByNickName(argv[3 + ac]);
							if (user) {
								channel->promoteUser(user->getNickName());
							}
							else {
								; //error: user not in channel
							}
							ac++;
						}
						else {
							; //error: missing nickname
						}
					}
					else if (flags[i] == 'l') {
						if (argv.size() > 3 + ac) {
							channel->setUsersLimit(std::atoi(argv[3 + ac].c_str()));
							channel->addMode('l');
							ac++;
						}
						else {
							; //error: missing users_limit
						}
					}
					else if (flags[i] == 'i' || flags[i] == 't' || flags[i] == 'k') {
						channel->addMode(flags[i]);
					}
					else {
						; //error: unknown flag
					}
				}
			}
			else if (flags[0] == '-') {
				for (size_t i = 1; i < flags.length(); i++) {
					if (flags[i] == 'o') {
						if (argv.size() > 3 + ac) {
							User * user = channel->getUserByNickName(argv[3 + ac]);
							if (user) {
								channel->demoteUser(user->getNickName());
							}
							else {
								; //error: user not in channel
							}
							ac++;
						}
						else {
							; //error: missing nickname
						}
					}
					else if (flags[i] == 'l' || flags[i] == 'i' || flags[i] == 't' || flags[i] == 'k') {
						channel->removeMode(flags[i]);
					}
					else {
						; //error: unknown flag
					}
				}
			}
			else {
				; //error: flags don't start with +/-
			}
		}
	}
	else {
		; //error: channel not found
	}
}

//==================================================

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

User *	Server::getUserByNickName(const std::string & nickName) const {
	std::map<int, User *>::const_iterator it = this->_users.begin();
	while (it != this->_users.end() && it->second->getNickName() != nickName) {
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