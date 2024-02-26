#include <iostream>
#include <map>
#include <poll.h>

class Channel;

class User
{
	private:
		User(void);
		pollfd								_pollFd;
		std::string							_userName;
		std::string							_nickName;
		//std::map<std::string, Channel *>	_channels;

	public:
		User(int fd);
		~User(void);

		//getters
		pollfd				getPollFd(void) const;
		
		const std::string &	getUserName(void) const;
		const std::string &	getNickName(void) const;

		//bool				isUserInChannel(std::string channelName) const;
		//bool				isUserOperator(std::string channelName) const;

		//void				addChannel(Channel * channel);
		//void				removeChannel(const std::string & channelName);
};
