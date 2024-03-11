#include <iostream>
#include <map>
#include <poll.h>

class Channel;

class User
{
	private:
		std::string							_userName;
		std::string							_nickName;
		//std::map<std::string, Channel *>	_channels;

		pollfd								_pollFd;
		bool								_isAuthenticated;
		bool								_isVerified;
		std::string							_message;
		
		User(void);

	public:
		User(int fd);
		~User(void);

		//getters
		bool				isAuthenticated(void) const;
		bool				isVerified(void) const;
		pollfd				getPollFd(void) const;
		int					getFd(void) const;
		short				getEvents(void) const;
		short				getRevents(void) const;
		
		const std::string &	getUserName(void) const;
		const std::string &	getNickName(void) const;
		const std::string &	getMessage(void) const;

		//setters
		void				authenticate(void);
		void				verify(void);
		void				setUserName(const std::string & userName);
		void				setNickName(const std::string & nickName);
		void				setMessage(const std::string & message);

		//UNUSED
		//bool				isUserInChannel(std::string channelName) const;
		//bool				isUserOperator(std::string channelName) const;
		//void				addChannel(Channel * channel);
		//void				removeChannel(const std::string & channelName);
};
