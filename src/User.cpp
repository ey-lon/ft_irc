#include "User.hpp"
#include "Channel.hpp"

//--------------------------------------------------
//constructors, destructors, ...
User::User(void) : _isAuthenticated(false), _isAuthorized(0) {}

User::~User() {}

User::User(int fd) : _isAuthenticated(false), _isAuthorized(0) {
	_pollFd.fd = fd;
	_pollFd.events = POLLIN;
	_pollFd.revents = 0;
	//_nickName = "abettini";
}

//--------------------------------------------------
//getters
bool	User::isAuthenticated(void) const {
	return (this->_isAuthenticated);
}

bool	User::isAuthorized(void) const {
	return (this->_isAuthorized);
}

pollfd	User::getPollFd(void) const {
	return (this->_pollFd);
}

int	User::getFd(void) const {
	return (this->_pollFd.fd);
}

short	User::getEvents(void) const {
	return (this->_pollFd.events);
}

short	User::getRevents(void) const {
	return (this->_pollFd.revents);
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

void	User::authorize() {
	this->_isAuthorized = true;
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

/* void	User::addChannel(Channel * channel)
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
