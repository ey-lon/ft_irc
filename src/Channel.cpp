#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(void) : _isInviteOnly(false)
{

}

Channel::~Channel(void)
{
	for (std::map<std::string, t_client *>::const_iterator it = _clients.begin(); it != this->_clients.end(); ++it) {
		delete (it->second);
	}
	_clients.clear();
}

Channel::Channel(const std::string & channelName) : _name(channelName), _isInviteOnly(false)
{

}

// getters

const std::string	Channel::getName(void) const
{
	return (this->_name);
}

bool	Channel::isInviteOnly(void) const
{
	return (this->_isInviteOnly);
}

bool	Channel::isClientInChannel(const std::string & clientUserName) const
{
	return (this->_clients.find(clientUserName) != this->_clients.end());
}

bool	Channel::isClientAdmin(const std::string & clientUserName) const
{
	std::map<std::string, t_client *>::const_iterator it = this->_clients.find(clientUserName);

	if (it != this->_clients.end()) {
		return (it->second->isAdmin);
	}
	return (false);
}

//

void	Channel::addClient(Client * client)
{
	if (client && this->_clients.find(client->getUserName()) == this->_clients.end())
	{
		t_client * newClient = new t_client;
		newClient->client = client;
		newClient->isAdmin = false;
		this->_clients.insert(std::make_pair(client->getUserName(), newClient));
	}
}

void	Channel::removeClient(const std::string & clientUserName)
{
	if (this->_clients.find(clientUserName) != this->_clients.end())
	{
		this->_clients[clientUserName]->client->removeChannel(this->_name);
		delete (this->_clients[clientUserName]);
		this->_clients.erase(clientUserName);
	}
}

void	Channel::promoteClient(const std::string & clientUserName)
{
	if (this->_clients.find(clientUserName) != this->_clients.end())
	{
		this->_clients[clientUserName]->isAdmin = true;
	}
}

void	Channel::demoteClient(const std::string & clientUserName)
{
	if (this->_clients.find(clientUserName) != this->_clients.end())
	{
		this->_clients[clientUserName]->isAdmin = false;
	}
}
