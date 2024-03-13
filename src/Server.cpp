#include "Server.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "Utils.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <string>

#define MSG (MSG_DONTWAIT | MSG_NOSIGNAL)
#define SRV_NAME "ircserv"

//--------------------------------------------------
//constructors, destructors, ...
Server::Server(void) : _serverName(SRV_NAME) {}

Server::Server(int port, const std::string & password) : _serverName(SRV_NAME) {
	if (port >= 0 && port <= USHRT_MAX) {
		this->_port = port;
	}
	else {
		throw ("invalid port");
	}
	if (isValidPassword(password)) {
		this->_password = password;
	}
	else {
		throw ("invalid password");
	}
}

Server::Server(const std::string & port, const std::string & password) : _serverName(SRV_NAME) {
	if (isValidPort(port)) {
		this->_port = std::atoi(port.c_str());
	}
	else {
		throw ("invalid port");
	}
	if (isValidPassword(password)) {
		this->_password = password;
	}
	else {
		throw ("invalid password");
	}
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

void	Server::pass(std::vector<std::string> argv, User * user) {
	//PASS <password>
	if (user->isVerified()) {
		; //error: user is already verified
	}
	else if (argv.size() < 2) {
		this->errorMsg(user, 464); //error: user didn't provide password
	}
	else if (argv[1] != this->_password) {
		this->errorMsg(user, 464); //error: password doesn't match
	}
	else {
		user->verify();
		//send message to client?
	}
}

//--------------------------------------------------
//authentication
void	Server::user(std::vector<std::string> argv, User * user) {
	//USER <username>
	if (!user->isVerified()) {
		this->errorMsg(user, 464); //error: user isn't verified
	}
	else if (user->isAuthenticated()) {
		this->errorMsg(user, 462); //error: user is already authenticated
	}
	else if (argv.size() < 2) {
		this->errorMsg(user, 461); //error: user didn't provide username
	}
	else if (!user->getUserName().empty()) {
		; //error: cannot change username
	}
	else if (getUserByUserName(argv[1])) {
		this->errorMsg(user, 400); //error: user already exists
	}
	else if (!isValidName(argv[1])) {
		this->errorMsg(user, 468); //error: invalid username
	}
	else {
		user->setUserName(argv[1]);
		if (!user->isAuthenticated() && !user->getNickName().empty()) {
			user->authenticate();
			this->welcomeMsg(user);
		}
	}
}

void	Server::nick(std::vector<std::string> argv, User * user) {
	//NICK <nickname>
	if (!user->isVerified()) {
		this->errorMsg(user, 464); //error: user isn't verified
	}
	else if (argv.size() < 2) {
		this->errorMsg(user, 431); //error: user didn't provide nickname
	}
	else if (getUserByNickName(argv[1])) {
		this->errorMsg(user, 433); //error: user already exists
	}
	else if (!isValidName(argv[1])) {
		this->errorMsg(user, 432); //error: invalid nickname
	}
	else {
		std::string rplNick = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " " + argv[1] + "\r\n";
		user->setNickName(argv[1]);
		if (user->isAuthenticated()) {
			this->serverMegaphone(NULL, rplNick);
		}
		else if (!user->getUserName().empty()) {
			user->authenticate();
			this->welcomeMsg(user);
		}
	}
}

//--------------------------------------------------
//other
void	Server::privmsg(std::vector<std::string> argv, User * user) {
	//PRIVMSG <nickname/channel> <message>
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451);
		return; //error: user isn't authenticated
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461);
		return ; //error: user didn't provide destination
	}
	if (argv.size() < 3 || argv[2].empty()) {
		this->errorMsg(user, 412);
		return ; //error: user didn't provide message
	}
	std::string	destName = argv[1];
	std::string msgText = argv[2];
	msgText = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " " + destName + " :" + msgText + "\r\n";
	if (destName[0] == '#') {
		//send msg to all user of channel
		destName.erase(0, 1);
		Channel * channelDest = this->getChannelByName(destName);
		if (!channelDest) {
			this->errorMsg(user, 403); //error: channel doesn't exist
		}
		else if (channelDest->hasFlag('n') && !channelDest->getUserByNickName(user->getNickName())) {
			this->errorMsg(user, 442); //error: user not in channel
		}
		else {
			this->channelMegaphone(channelDest, user, msgText);
		}
	}
	else {
		//send message to user
		User * userDest = this->getUserByNickName(destName);
		if (!userDest) {
			this->errorMsg(user, 401); //error: user_to_text not found
		}
		else if (!userDest->isAuthenticated()) {
			; //error: user_to_text isn't authenticated
		}
		else if (userDest != user) {
			send(userDest->getFd(), msgText.c_str(), msgText.length(), MSG);
		}
	}
}

void	Server::join(std::vector<std::string> argv, User * user) {
	//JOIN <ch1,ch2,...,chn> [<key1,key2,...,keyn>]
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451);
		return; //error: user isn't authenticated
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461);
		return; //error: user didn't provide channel(s)
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
			if (channel->getUserByNickName(user->getNickName())) {
				this->errorMsg(user, 443); //error: user is already in channel
			}
			else if (channel->hasFlag('i')) {
				this->errorMsg(user, 473); //error: channel is invite only
			}
			else if (channel->hasFlag('k') && (i >= keyVec.size() || channel->getPassword() != keyVec[i])) {
				this->errorMsg(user, 475); //error: password needed, user didn't send it or it doesn't match
			}
			else if (channel->hasFlag('l') && channel->nUsers() >= channel->getUsersLimit()) {
				this->errorMsg(user, 471); //error: too many users
			}
			else { //user can join
				channel->addUser(user);
				if (channel->nUsers() == 1) {
					channel->promoteUser(user->getNickName());
				}
				this->joinMsg(channel, user); //send JOIN messages to users
			}
		}
		else if (isValidName(channelVec[i])) {	// <-- channel doesn't exist
			if (i < keyVec.size() && !keyVec[i].empty() && keyVec[i] != ".") {
				if (isValidPassword(keyVec[i])) {
					channel = createChannel(channelVec[i]);
					channel->setPassword(keyVec[i]);
					channel->addMode('k');
				}
				else {
					this->errorMsg(user, 400); // error: invalid password <-- won't create channel
				}
			}
			else {
				channel = createChannel(channelVec[i]);
			}
			if (channel) {
				channel->addUser(user);
				channel->promoteUser(user->getNickName());
				this->joinMsg(channel, user); //send JOIN messages to users
			}
		}
		else {
			this->errorMsg(user, 403); //error: channel doesn't exist but user provided invalid name 
		}
	}
}

void	Server::part(std::vector<std::string> argv, User * user) {
	//PART <channel> [<message>]
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461); //error: user did't provide channel
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') {
		channelName.erase(0, 1);
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); //error: user did't provide channel
	}
	else if (!channel->getUserByNickName(user->getNickName())) {
		this->errorMsg(user, 442); //error: user not in channel
	}
	else {
		std::string rplMsg = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " #" + channelName;
		if (argv.size() > 2) {
			rplMsg += " :" + argv[2] + "\r\n";
		}
		rplMsg += "\r\n";
		this->channelMegaphone(channel, NULL, rplMsg);
		channel->removeUser(user->getNickName());
	}
}

void	Server::quit(std::vector<std::string> argv, User * user) {
	//QUIT [<message>]
	if (user->isAuthenticated()) {
		std::string rplMsg = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0];
		if (argv.size() > 1) {
			rplMsg += " :" + argv[1];
		}
		rplMsg += "\r\n";
		this->serverMegaphone(NULL, rplMsg);
	}
	this->removeUser(user->getFd());
}

void	Server::ping(std::vector<std::string> argv, User * user) {
	std::string rplMsg = "PONG";
	if (argv.size() > 1) {
		rplMsg += " " + argv[1];
	}
	rplMsg += "\r\n";
	send(user->getFd(), rplMsg.c_str(), rplMsg.length(), MSG);
}

//--------------------------------------------------
//operators only
void	Server::kick(std::vector<std::string> argv, User * user) {
	//KICK <#channel> <nickname> [<message>]
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return;
	}
	if (argv.size() < 3) {
		this->errorMsg(user, 461); //error: user didn't provide channel and/or nickname
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	Channel * channel = getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403);
	}
	else {
		std::string nickName = argv[2];
		if (!channel->isUserOperator(user->getNickName())) {
			this->errorMsg(user, 482); //error: user is not operator
		}
		else {
			std::string rplKick = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " #" + channelName + " " + argv[2];
			if (argv.size() > 3 && !argv[3].empty()) {
				rplKick += " :" + argv[3]; //append optional message
			}
			rplKick += "\r\n";
			this->channelMegaphone(channel, NULL, rplKick);
			channel->removeUser(nickName);
		}
	}
}

void	Server::invite(std::vector<std::string> argv, User * user) {
	//INVITE <nickname> <channel>
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return ;
	}
	if (argv.size() < 3) {
		this->errorMsg(user, 461); //error: user didn't provide nickname and/or channel 
		return ;
	}
	std::string channelName = argv[2];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); //error: channel not found
	}
	else if (!channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); //error: user is not operator
	}
	else if (channel->hasFlag('l') && channel->nUsers() >= channel->getUsersLimit()) {
		this->errorMsg(user, 471); //error: too many users
	}
	else {
		std::string nickName = argv[1];
		User * userInv = getUserByNickName(nickName);
		if (!userInv) {
			this->errorMsg(user, 401); //error: user_to_invite not found
		}
		else if (!userInv->isAuthenticated()) {
			; //error: user_to_invite isn't authenticated
		}
		else if (channel->getUserByNickName(nickName)) {
			this->errorMsg(user, 443); //error :user_to_invite is already in channel
		}
		else {
			channel->addUser(userInv);
			this->joinMsg(channel, userInv);
		}			
	}
}

void	Server::topic(std::vector<std::string> argv, User * user) {
	//TOPIC <channel> [<topic>]
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461); //error: user didn't send channel
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); //error: channel not found
	}
	else if (channel->hasFlag('t') && !channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); //error: restrictions are set but user is not operator
	}
	else if (argv.size() > 2) { 		//<-- set topic
		channel->setTopic(argv[2]);
		std::string rplTopic;
		rplTopic = ":" + user->getNickName() + "!" + this->getName() + " " + argv[0] + " #" + channel->getName() + " :" + channel->getTopic() + "\r\n";
		this->channelMegaphone(channel, NULL, rplTopic);
	}
	else {								//<-- view topic
		std::string rplTopic;
		if (channel->getTopic().empty()) {
			rplTopic = ":" + this->getName() + " 331 " + user->getNickName() + " #" + channel->getName() + " :No topic is set" + "\r\n";
		}
		else {
			rplTopic = ":" + this->getName() + " 332 " + user->getNickName() + " #" + channel->getName() + " :" + channel->getTopic() + "\r\n";
		}
		send(user->getFd(), rplTopic.c_str(), rplTopic.length(), MSG);
	}
}

void	Server::mode(std::vector<std::string> argv, User * user) {
	//MODE <channel> {[+|-]|i|t|k|o|l} [<user>] [<limit>] [<password>]
	if (!user->isAuthenticated()) {
		this->errorMsg(user, 451); //error: user isn't authenticated
		return;
	}
	if (argv.size() < 2) {
		this->errorMsg(user, 461); //error: user didn't send channel or flags
		return ;
	}
	std::string channelName = argv[1];
	if (channelName[0] == '#') { // <-- if there's no # symbol then error?
		channelName.erase(0, 1); // remove #
	}
	Channel * channel = this->getChannelByName(channelName);
	if (!channel) {
		this->errorMsg(user, 403); //error: channel not found
	}
	else if (!channel->isUserOperator(user->getNickName())) {
		this->errorMsg(user, 482); //error: user is not operator
	}
	else if (argv.size() < 3) { // <-- view mode
		std::string rplMode = ":" + this->getName() + " 324 " + user->getNickName() + " #" + channelName + " +" + channel->getMode();
		for (size_t i = 0; i < channel->getMode().length(); ++i) {
			if (channel->getMode()[i] == 'l') {
				rplMode += " " + toString(channel->getUsersLimit());
			}
			else if (channel->getMode()[i] == 'k' && channel->getPassword().length() > 0) {
				rplMode += " " + channel->getPassword();
			}
		}
		rplMode += "\r\n";
		send(user->getFd(), rplMode.c_str(), rplMode.length(), MSG);
	}
	else if (argv[2][0] != '+' && argv[2][0] != '-') {
		this->errorMsg(user, 472); //error: flags dont start with +/-
	}
	else {
		std::string flags = argv[2];
		size_t argIndex = 3;
		for (size_t i = 1; i < flags.length(); i++) {
			if (flags[i] == 'o') {
				if (argv.size() > argIndex) {
					if (channel->getUserByNickName(argv[argIndex])) {
						if (flags[0] != '-') {
							channel->promoteUser(argv[argIndex]);
						}
						else {
							channel->demoteUser(argv[argIndex]);
						}
						std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + " " + argv[argIndex] + "\r\n"; 
						this->channelMegaphone(channel, NULL, rplMsg);
					}
					else {
						this->errorMsg(user, 441); //error: user not in channel
					}
					argIndex++;
				}
				else {
					this->errorMsg(user, 461); //error: missing nickname
				}
			}
			else if (flags[i] == 'l') {
				if (flags[0] != '-') {
					if (argv.size() > argIndex) {
						if (std::atoi(argv[argIndex].c_str()) > 0) {
							channel->setUsersLimit(std::atoi(argv[argIndex].c_str()));
							channel->addMode('l');
							std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + " " + argv[argIndex] + "\r\n"; 
							this->channelMegaphone(channel, NULL, rplMsg);
						}
						else {
							this->errorMsg(user, 400); //error: invalid users_limit
						}
						argIndex++;
					}
					else {
						this->errorMsg(user, 461); //error: missing users_limit
					}
				}
				else {
					channel->removeMode(flags[i]);
					std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + "\r\n"; 
					this->channelMegaphone(channel, NULL, rplMsg);
				}
			}
			else if (flags[i] == 'k') {
				if (flags[0] != '-') {
					if (argv.size() > argIndex) {
						if (isValidPassword(argv[argIndex])) {
							channel->setPassword(argv[argIndex]);
							channel->addMode('k');
							std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + " " + argv[argIndex] + "\r\n"; 
							this->channelMegaphone(channel, NULL, rplMsg);
						}
						else {
							this->errorMsg(user, 400); //error: invalid password
						}
						argIndex++;
					}
					else {
						this->errorMsg(user, 461); //error: missing password
					}
				}
				else {
					channel->removeMode(flags[i]);
					std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + "\r\n"; 
					this->channelMegaphone(channel, NULL, rplMsg);
				}
			}
			else if (flags[i] == 'i' || flags[i] == 't' || flags[i] == 'n') {
				if (flags[0] != '-') {
					channel->addMode(flags[i]);
				}
				else {
					channel->removeMode(flags[i]);
				}
				std::string rplMsg = ":" + this->getName() + " " + argv[0] + " #" + channel->getName() + " " + flags[0] + flags[i] + "\r\n"; 
				this->channelMegaphone(channel, NULL, rplMsg);
			}
			else if (flags[i] == 'b') {
				; //ignore flag b
			}
			else {
				this->errorMsg(user, 472); //error: unknown flag
			}
		}
	}
}

//==================================================

void	Server::channelMegaphone(Channel * channel, User * user, const std::string & msg) const {
	//send a message to all users in channel
	if (!channel) {
		return;
	}
	// user is to be ignored if not NULL
	for (std::map<User *, bool>::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
		if (!user || user != it->first) {
			send(it->first->getFd(), msg.c_str(), msg.length(), MSG);
		}
	}
}

void	Server::serverMegaphone(User * user, const std::string & msg) const {
	//send a message to all users in server
	// user is to be ignored if not NULL
	for (std::map<int, User *>::const_iterator it = this->_users.begin(); it != _users.end(); ++it) {
		if (!user || user != it->second) {
			send(it->second->getFd(), msg.c_str(), msg.length(), MSG);
		}
	}
}

void Server::joinMsg(Channel *channel, User *user) {
	std::string rplJoin = ":" + user->getNickName() + "!" + this->getName() + " JOIN #" + channel->getName() + "\r\n";
    std::string	rplUserList = ":" + this->getName() + " 353 " + user->getNickName() + " = #" + channel->getName() + " :";
	//user_list
	for (std::map<User *, bool>::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
		send(it->first->getFd(), rplJoin.c_str(), rplJoin.length(), MSG);
		rplUserList += it->first->getNickName();
		if(it != --channel->getUsers().end()) {
			rplUserList += " ";
		}
	}
	rplUserList += "\r\n";
	//topic
	std::string rplTopic;
	if (channel->getTopic().empty()) {
		rplTopic = ":" + this->getName() + " 331 " + user->getNickName() + " #" + channel->getName() + " :No topic is set" + "\r\n";
	}
	else {
		rplTopic = ":" + this->getName() + " 332 " + user->getNickName() + " #" + channel->getName() + " :" + channel->getTopic() + "\r\n";
	}
	//mode
    std::string rplMode = ":" + this->getName() + " 324 " + user->getNickName() + " #" + channel->getName() + " +" + channel->getMode();
	for (size_t i = 0; i < channel->getMode().length(); ++i) {
		if (channel->getMode()[i] == 'l') {
			rplMode += " " + toString(channel->getUsersLimit());
		}
		else if (channel->getMode()[i] == 'k' && channel->getPassword().length() > 0) {
			rplMode += " " + channel->getPassword();
		}
	}
	rplMode += "\r\n";

	send(user->getFd(), rplTopic.c_str(), rplTopic.length(), MSG);
    send(user->getFd(), rplMode.c_str(), rplMode.length(), MSG);
	send(user->getFd(), rplUserList.c_str(), rplUserList.length(), MSG);
	
	for (std::map<User *, bool>::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
		if (channel->isUserOperator(it->first->getNickName())) {
   			std::string rplOpList = ":" + this->getName() + " MODE #" + channel->getName() + " +o " + it->first->getNickName() + "\r\n";
			send(user->getFd(), rplOpList.c_str(), rplOpList.length(), MSG);
		}
	}
}

void	Server::welcomeMsg(User * user) {
	std::string nickname = user->getNickName();
	std::string RPL_WELCOME =	":" + this->getName() + " 001 " + nickname + " :Welcome to the Internet Relay Network " + nickname + "\r\n";
	std::string RPL_YOURHOST =	":" + this->getName() + " 002 " + nickname + " :Your host is " + this->_hostname + ", running version 42 \r\n";
	std::string RPL_CREATED = 	":" + this->getName() + " 003 " + nickname + " :This server was created 30/10/2023\r\n";
	std::string RPL_MYINFO = 	":" + this->getName() + " 004 " + nickname + " ircserv 42 +o +l+i+k+t+n\r\n";
	std::string RPL_ISUPPORT =	":" + this->getName() + " 005 " + nickname + " operator ban limit invite key topic :are supported by this server\r\n";
	std::string RPL_MOTD =		":" + this->getName() + " 372 " + nickname + " :Welcome to " + this->getName() + "\r\n";
	std::string RPL_ENDOFMOTD =	":" + this->getName() + " 376 " + nickname + " :End of MOTD command\r\n";

	int fd = user->getFd();
	send(fd, RPL_WELCOME.c_str(), RPL_WELCOME.length(), MSG);
	send(fd, RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), MSG);
	send(fd, RPL_CREATED.c_str(), RPL_CREATED.length(), MSG);
	send(fd, RPL_MYINFO.c_str(), RPL_MYINFO.length(), MSG);
	send(fd, RPL_ISUPPORT.c_str(), RPL_ISUPPORT.length(), MSG);
	send(fd, RPL_MOTD.c_str(), RPL_MOTD.length(), MSG);
	send(fd, RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), MSG);
}

void	Server::errorMsg(User * user, int code) {
	//:<serv_name> <error_code> <nickname> :<message>

	std::string rplErr = ":" + this->getName() + " " + toString(code);
	if (!user->getNickName().empty()) {
		rplErr += " " + user->getNickName();
	}
	rplErr += " :" + _errors[code] + "\r\n";
	send (user->getFd(), rplErr.c_str(), rplErr.length(), MSG);
}

void	Server::dealCommand(std::vector<std::string> argv, User *user) {
	if (argv.empty()) {
		return;
	}
	else if (argv[0] == "PING") {
		this->ping(argv, user);
	}
	else if (argv[0] == "QUIT") {
		this->quit(argv, user);
	}
	else if (argv[0] == "PASS") {
		this->pass(argv, user);
	}
	else if (argv[0] == "USER") {
		this->user(argv, user);
	}
	else if (argv[0] == "NICK") {
		this->nick(argv, user);
	}
	else if (argv[0] == "JOIN") {
		this->join(argv, user);
	}
	else if (argv[0] == "PART") {
		this->part(argv, user);
	}
	else if (argv[0] == "PRIVMSG") {
		this->privmsg(argv, user);
	}
	else if (argv[0] == "INVITE") {
		this->invite(argv, user);
	}
	else if (argv[0] == "TOPIC") {
		this->topic(argv, user);
	}
	else if (argv[0] == "KICK") {
		this->kick(argv, user);
	}
	else if (argv[0] == "MODE") {
		this->mode(argv, user);
	}
	else if (argv[0] != "CAP" && argv[0] != "WHO" && argv[0] != "USERHOST") {
		this->errorMsg(user, 421);
	}
}

int Server::dealMessage(int userFd) {
	User * ptr = this->getUserByFd(userFd);
	if (!ptr) {	// <-- don't know how it could happen, but you never know.
		return (-1);
	}
    char	buffer[1024];
    ssize_t bytesRead = recv(userFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
		this->removeUser(userFd);
		return (1);
    }
	buffer[bytesRead] = '\0'; // <--- NECESSARY
	//std::cout << "bytes received = " << bytesRead << ", " << buffer << std::endl; 
	ptr->setMessage(ptr->getMessage() + buffer);
	if (ptr->getMessage().find('\n') == std::string::npos) {
		return (0);
	}
	std::cout << "[" << ptr->getFd() << "]: " << ptr->getMessage() << std::endl;
	std::vector<std::string> argv = parseInput(ptr->getMessage());
	this->dealCommand(argv, ptr);
	if (!argv.empty() && argv[0] == "QUIT") {
		return (1);
	}
	ptr->setMessage("");
	return (0);
}

void	Server::run(void) {
	std::cout << "Server running at " << _ip << ":" << _port << std::endl;

	pollfd	serverPollFd;
	serverPollFd.fd = this->_serverSocket;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	this->_fds.push_back(serverPollFd);

	this->_isRunning = true;
	while (this->isRunning() == true) {
		poll(this->_fds.data(), this->_fds.size(), -1);
		for (size_t i = 0; i < this->_fds.size(); ++i) {
			if (this->_fds[i].revents & POLLIN) {
				if (this->_fds[i].fd == this->_serverSocket) {
					this->createUser();
				}
				else if (this->dealMessage(this->_fds[i].fd) == 1) {
					--i; // <-- if client disconnects, its pollfd is removed from the vector, so the next pollfd is at the index of the one that got removed. 
				}
			}
		}
	}
}

void	Server::init(void) {
	// Create a socket
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1) {
		throw ("socket creation failure");
    }
	// Set socket options
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw ("socket option settings failure");
    }
	int recicle = -1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &recicle, sizeof(recicle)) == -1) {
		throw ("socket reusable settings failure");
	}
	// Get hostname
	if (gethostname(_hostname, sizeof(_hostname)) == -1) {
		throw ("gethostname failure");
	}
	// Get ip
	struct hostent *host_entry = gethostbyname(_hostname);
	if (host_entry == NULL) {
		throw ("gethostbyname failure");
	}
	_ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    // Set up server address structure
	_serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = INADDR_ANY;	// Listen on all available interfaces
    _serverAddress.sin_port = htons(this->_port);
	// Controls on fd
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0) {
		throw ("non-blocking socket setting failure");
	}
	// Bind the socket
    if (bind(_serverSocket, (struct sockaddr*)&_serverAddress, sizeof(_serverAddress)) == -1) {
        throw ("socket binding failure");
    }
    // Listen for incoming connections
    if (listen(_serverSocket, SOMAXCONN) == -1) {
        throw ("socket listening failure");
    }
}

//--------------------------------------------------
//getters
const std::string &	Server::getName(void) const {
	return (this->_serverName);
}

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

bool	Server::isRunning(void) const {
	return (this->_isRunning);
}

//--------------------------------------------------
//setters
void	Server::setPassword(const std::string &password) {
	this->_password = password;
}

void	Server::stop(void) {
	this->_isRunning = false;
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

User *	Server::createUser() {
	int userFd = accept(this->_serverSocket, NULL, NULL);
	if (userFd == -1) {
		std::cerr << "Connection error" << std::endl;
		return (NULL);
	}
	std::cout << "New connection accepted, user_fd [" << userFd << "]" << std::endl;
	if (this->_users.find(userFd) == this->_users.end()) {
		User * newUser = new User(userFd);
		this->_users.insert(std::make_pair(userFd, newUser));		// <-- add user to map
		this->_fds.push_back(newUser->getPollFd());					// <-- add user pollfd to vector
		return (newUser);
	}
	else {
		return (NULL);
	}
}

void	Server::removeUser(int userFd) {
	if (this->_users.find(userFd) != this->_users.end()) {

 		//remove user from channels
		std::string nickName = this->getUserByFd(userFd)->getNickName();
		for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++) {
			it->second->removeUser(nickName);
		}

		delete (this->_users[userFd]);								// <-- delete User *
		this->_users.erase(userFd);									// <-- remove user from map

		std::vector<pollfd>::iterator it = this->_fds.begin();
		while (it != this->_fds.end() && it->fd != userFd) {
			++it;
		}
		if (it != this->_fds.end()) {
			this->_fds.erase(it);									// <-- remove user pollfd from vector
		}

		std::cout << "Connection with user_fd [" << userFd << "] terminated." << std::endl;
	}
}
