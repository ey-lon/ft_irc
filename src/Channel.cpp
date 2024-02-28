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

bool	Channel::isInviteOnly(void) const {
	return (this->_isInviteOnly);
}

//--------------------------------------------------
//setters
void	Channel::setName(const std::string & name) {
	this->_name = name;
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
