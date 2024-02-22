#include "Client.hpp"
#include "Channel.hpp"

Client::Client(void)
{

}

Client::~Client()
{

}

Client::Client(int fd)
{
	_pollFd.fd = fd;
	_pollFd.events = POLLIN;
	_pollFd.revents = 0;
}

//getters

int	Client::getFd() const
{
	return (this->_pollFd.fd);
}

short	Client::getEvents() const
{
	return (this->_pollFd.events);
}

short	Client::getRevents() const
{
	return (this->_pollFd.revents);
}

const std::string &	Client::getUserName() const
{
	return (this->_userName);
}

const std::string &	Client::getNickName() const
{
	return (this->_nickName);
}

bool	Client::isClientInChannel(std::string channelName) const
{
	return (this->_channels.find(channelName) != this->_channels.end());
}

bool	Client::isClientAdmin(std::string channelName) const
{
	std::map<std::string, Channel *>::const_iterator it = this->_channels.find(channelName);

	if (it != this->_channels.end()) {
		return (it->second->isClientAdmin(this->_userName));
	}
	return (false);
}

//
void	Client::addChannel(Channel * channel)
{
	if (channel && this->_channels.find(channel->getName()) == this->_channels.end())
	{
		this->_channels.insert(std::make_pair(channel->getName(), channel));
	}
}

void	Client::removeChannel(const std::string & channelName)
{
	if (this->_channels.find(channelName) != this->_channels.end())
	{
		this->_channels.erase(channelName);
	}
}
