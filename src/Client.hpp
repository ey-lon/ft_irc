#include <iostream>
#include <map>
#include <poll.h>

class Channel;

class Client
{
	private:
		pollfd								_pollFd;
		std::string							_userName;
		std::string							_nickName;
		std::map<std::string, Channel *>	_channels;

	public:
		Client(void);
		~Client(void);

		Client(int fd);

		//getters
		int					getFd() const;
		short				getEvents() const;
		short				getRevents() const;
		
		const std::string &	getUserName() const;
		const std::string &	getNickName() const;

		bool				isClientInChannel(std::string channelName) const;
		bool				isClientAdmin(std::string channelName) const;

		//
		void				addChannel(Channel * channel);
		void				removeChannel(const std::string & channelName);
};
