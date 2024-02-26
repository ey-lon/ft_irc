#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(void) : _isInviteOnly(false)
{

}

Channel::~Channel(void)
{
	for (std::map<std::string, t_user *>::const_iterator it = _users.begin(); it != this->_users.end(); ++it) {
		delete (it->second);
	}
	_users.clear();
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

bool	Channel::isUserPresent(const std::string & userUserName) const
{
	return (this->_users.find(userUserName) != this->_users.end());
}

bool	Channel::isUserOperator(const std::string & userUserName) const
{
	std::map<std::string, t_user *>::const_iterator it = this->_users.find(userUserName);

	if (it != this->_users.end()) {
		return (it->second->isOperator);
	}
	return (false);
}

//

void	Channel::addUser(User * user)
{
	if (user && this->_users.find(user->getUserName()) == this->_users.end())
	{
		t_user * newUser = new t_user;
		newUser->user = user;
		newUser->isOperator = false;
		this->_users.insert(std::make_pair(user->getUserName(), newUser));
	}
}

void	Channel::removeUser(const std::string & userUserName)
{
	if (this->_users.find(userUserName) != this->_users.end())
	{
		//this->_users[userUserName]->user->removeChannel(this->_name);
		delete (this->_users[userUserName]);
		this->_users.erase(userUserName);
	}
}

void	Channel::promoteUser(const std::string & userUserName)
{
	if (this->_users.find(userUserName) != this->_users.end())
	{
		this->_users[userUserName]->isOperator = true;
	}
}

void	Channel::demoteUser(const std::string & userUserName)
{
	if (this->_users.find(userUserName) != this->_users.end())
	{
		this->_users[userUserName]->isOperator = false;
	}
}
