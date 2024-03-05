#include "Channel.hpp"
#include "User.hpp"

//--------------------------------------------------
//constructors, destructors, ...
Channel::Channel(void) : _usersLimit(0) {}

Channel::Channel(const std::string & channelName) : _name(channelName), _usersLimit(0) {}

Channel::~Channel(void) {
	for (std::map<User *, bool>::const_iterator it = _users.begin(); it != this->_users.end(); ++it) {
		delete (it->first);
	}
	_users.clear();
}

//--------------------------------------------------
// getters
const std::string	Channel::getName(void) const {
	return (this->_name);
}

const std::string	Channel::getPassword(void) const {
	return (this->_password);
}

const std::string	Channel::getTopic(void) const {
	return (this->_topic);
}

const std::string	Channel::getMode(void) const {
	return (this->_mode);
}

int	Channel::getUsersLimit(void) const {
	return (_usersLimit);
}

int	Channel::nUsers(void) const {
	return (_users.size());
}

bool	Channel::hasFlag(char flag) const {
	return (this->_mode.find(flag) != std::string::npos);
}

//--------------------------------------------------
//setters
void	Channel::setName(const std::string & name) {
	this->_name = name;
}

void	Channel::setPassword(const std::string & password) {
	this->_password = password;
}

void	Channel::setTopic(const std::string & topic) {
	this->_topic = topic;
}

void	Channel::addMode(const std::string & mode) {
	for (size_t i = 0; i < mode.length(); i++) {
		if (this->_mode.find(mode[i]) == std::string::npos) {
			this->_mode.push_back(mode[i]);
		}
	}
}

void	Channel::removeMode(const std::string & mode) {
	for (size_t i = 0; i < mode.length(); i++) {
		if (this->_mode.find(mode[i]) != std::string::npos) {
			this->_mode.erase(mode[i]);
		}
	}
}

//--------------------------------------------------
//users
User *	Channel::getUserByNickName(const std::string & nickName) const {
	std::map<User *, bool>::const_iterator it = _users.begin();
	while (it != this->_users.end() && it->first->getNickName() != nickName) {
		++it;
	}
	if (it != this->_users.end()) {
		return (it->first);
	}
	else {
		return (NULL);
	}
}

bool	Channel::isUserPresent(const std::string & nickName) const {
	std::map<User *, bool>::const_iterator it = _users.begin();
	while (it != this->_users.end() && it->first->getNickName() != nickName) {
		++it;
	}
	return (it != this->_users.end());
}

bool	Channel::isUserOperator(const std::string & nickName) const {
	std::map<User *, bool>::const_iterator it = _users.begin();
	while (it != this->_users.end() && it->first->getNickName() != nickName) {
		++it;
	}
	return (it != this->_users.end() && it->second);
}

void	Channel::addUser(User * user) {
	if (user && this->_users.find(user) == this->_users.end()) 	{
		this->_users.insert(std::make_pair(user, false));
	}
}

void	Channel::removeUser(const std::string & nickName) {
	std::map<User *, bool>::iterator it = _users.begin();
	while (it != this->_users.end() && it->first->getNickName() != nickName) {
		++it;
	}
	if (it != _users.end()) {
		this->_users.erase(it);
	}
}

void	Channel::promoteUser(const std::string & nickName) {
	std::map<User *, bool>::iterator it = _users.begin();
	while (it != this->_users.end() && it->first->getNickName() != nickName) {
		++it;
	}
	if (it != _users.end()) {
		it->second = true;;
	}
}

void	Channel::demoteUser(const std::string & nickName) {
	std::map<User *, bool>::iterator it = _users.begin();
	while (it != this->_users.end() && it->first->getNickName() != nickName) {
		++it;
	}
	if (it != _users.end()) {
		it->second = false;;
	}
}

/* 
void Channel::sendJoinMessageBack(const std::string & username) {
	std::string RPL_JOIN = ":lamici!irc_serv JOIN #canale\r\n";
    std::string RPL_NAMREPLY = ":ircserv 353 lamici = #canale :lamici\r\n";
    std::string RPL_WHOISOPERATOR = ":ircserv MODE #canale +o lamici\r\n";
    std::string RPL_CHANNELMODEIS = ":ircserv 324 lamici #canale +t\r\n";
    send(user->getFd(), RPL_JOIN.c_str(), RPL_JOIN.length(), MSG);
    send(user->getFd(), RPL_CHANNELMODEIS.c_str(), RPL_CHANNELMODEIS.length(), MSG);
    send(user->getFd(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), MSG);
    send(user->getFd(), RPL_WHOISOPERATOR.c_str(), RPL_WHOISOPERATOR.length(), MSG);
} */