#include "User.hpp"
#include "Channel.hpp"

User::User(void)
{

}

User::~User()
{

}

User::User(int fd)
{
	_pollFd.fd = fd;
	_pollFd.events = POLLIN;
	_pollFd.revents = 0;
	//_nickName = "abettini";
}

//getters

pollfd	User::getPollFd(void) const
{
	return (this->_pollFd);
}

const std::string &	User::getUserName(void) const
{
	return (this->_userName);
}

const std::string &	User::getNickName(void) const
{
	return (this->_nickName);
}

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
