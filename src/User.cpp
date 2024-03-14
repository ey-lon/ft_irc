#include "User.hpp"
#include "Channel.hpp"
#include <unistd.h>

//--------------------------------------------------
//constructors, destructors, ...
User::User(void) : _isAuthenticated(false), _isVerified(false) {}

User::~User() {
	close(this->getFd());
}

User::User(int fd) : _isAuthenticated(false), _isVerified(false) {
	this->_fd = fd;
}

//--------------------------------------------------
//getters
bool	User::isAuthenticated(void) const {
	return (this->_isAuthenticated);
}

bool	User::isVerified(void) const {
	return (this->_isVerified);
}

int	User::getFd(void) const {
	return (this->_fd);
}


const std::string &	User::getUserName(void) const {
	return (this->_userName);
}

const std::string &	User::getNickName(void) const {
	return (this->_nickName);
}

const std::string &	User::getMessage(void) const {
	return (this->_message);
}

//--------------------------------------------------
//setters
void	User::authenticate() {
	this->_isAuthenticated = true;
}

void	User::verify() {
	this->_isVerified = true;
}

void	User::setUserName(const std::string & userName) {
	this->_userName = userName;
}

void	User::setNickName(const std::string & nickName) {
	this->_nickName = nickName;
}

void	User::setMessage(const std::string & message) {
	this->_message = message;
}

//--------------------------------------------------
//UNUSED
/* bool	User::isUserInChannel(std::string channelName) const
{
	return (this->_channels.find(channelName) != this->_channels.end());
} */

/* bool	User::isUserOperator(std::string channelName) const
{
	std::map<std::string, Channel *>::const_iterator it = this->_channels.find(channelName);

	if (it != this->_channels.end()) {
		return (it->second->isUserOperator(this->_userName));
	}
	return (false);
} */

/*  void	User::addChannel(Channel * channel)
{
	if (channel && this->_channels.find(channel->getName()) == this->_channels.end())
	{
		this->_channels.insert(std::make_pair(channel->getName(), channel));
	}
} */

/* void	User::removeChannel(const std::string & channelName)
{
	if (this->_channels.find(channelName) != this->_channels.end())
	{
		this->_channels.erase(channelName);
	}
} */
