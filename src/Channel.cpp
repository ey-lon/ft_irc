#include "Channel.hpp"
#include "User.hpp"

//--------------------------------------------------
//constructors, destructors, ...
Channel::Channel(void) : _isInviteOnly(false) {}

Channel::Channel(const std::string & channelName) : _name(channelName), _isInviteOnly(false) {}

Channel::~Channel(void) {
	for (std::map<std::string, t_user *>::const_iterator it = _users.begin(); it != this->_users.end(); ++it) {
		delete (it->second);
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

bool	Channel::isInviteOnly(void) const {
	return (this->_isInviteOnly);
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

void	Channel::setInviteOnly(bool isInviteOnly) {
	this->_isInviteOnly = isInviteOnly;
}

//--------------------------------------------------
//users
bool	Channel::isUserPresent(const std::string & userName) const {
	return (this->_users.find(userName) != this->_users.end());
}

bool	Channel::isUserOperator(const std::string & userName) const {
	std::map<std::string, t_user *>::const_iterator it = this->_users.find(userName);
	if (it != this->_users.end()) {
		return (it->second->isOperator);
	}
	return (false);
}

void	Channel::addUser(User * user) {
	if (user && this->_users.find(user->getUserName()) == this->_users.end()) 	{
		t_user * newUser = new t_user;
		newUser->userPtr = user;
		newUser->isOperator = false;
		this->_users.insert(std::make_pair(user->getUserName(), newUser));
	}
}

void	Channel::removeUser(const std::string & userName) {
	if (this->_users.find(userName) != this->_users.end()) 	{
		//this->_users[userName]->userPtr->removeChannel(this->_name);
		delete (this->_users[userName]);
		this->_users.erase(userName);
	}
}

void	Channel::promoteUser(const std::string & userName) {
	if (this->_users.find(userName) != this->_users.end()) {
		this->_users[userName]->isOperator = true;
	}
}

void	Channel::demoteUser(const std::string & userName) {
	if (this->_users.find(userName) != this->_users.end()) {
		this->_users[userName]->isOperator = false;
	}
}

void Channel::sendJoinMessageBack(const std::string & username) {
	std::string RPL_JOIN = ":lamici!irc_serv JOIN #canale\r\n";
    std::string RPL_NAMREPLY = ":ircserv 353 lamici = #canale :lamici\r\n";
    std::string RPL_WHOISOPERATOR = ":ircserv MODE #canale +o lamici\r\n";
    std::string RPL_CHANNELMODEIS = ":ircserv 324 lamici #canale +t\r\n";
    send(user->getFd(), RPL_JOIN.c_str(), RPL_JOIN.length(), MSG);
    send(user->getFd(), RPL_CHANNELMODEIS.c_str(), RPL_CHANNELMODEIS.length(), MSG);
    send(user->getFd(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), MSG);
    send(user->getFd(), RPL_WHOISOPERATOR.c_str(), RPL_WHOISOPERATOR.length(), MSG);
}