#include <map>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>

#define MSG (MSG_DONTWAIT | MSG_NOSIGNAL)

class Channel;

class User;

class Server
{
	private:
		std::map<std::string, Channel *>	_channels;
		std::map<std::string, User *>		_users;
		std::string							_password;
		int									_port;

		int									_serverSocket;
		sockaddr_in							_serverAddress;

		char								_hostname[256];
		std::vector<pollfd>					_fds;

	public:
		Server(void);
		~Server(void);

		Server(int port, const std::string & password);

		void				start(void);
		void				loop(void);

		void				newConnection(void);
		void				dealMessage(int);

		//getters
		const std::string &	getPassword(void) const;
		int					getPort(void) const;
		int					nUsers(void) const;
		int					nChannels(void) const;

		User *				getUserByName(const std::string & userName) const;
		User *				getUserByFd(int userFd) const;

		Channel *			getChannelByName(const std::string & channelName) const;

		//setters
		void				setPassword(const std::string & password);

		//channels
		void				createChannel(const std::string & channelName);
		void				addChannel(Channel * channel);
		void				removeChannel(const std::string & channelName);

		//users
		void				createUser(int fd, const std::string & userName);
		void				addUser(User * user);
		void				removeUser(const std::string & userName);
		void				removeUser(int userFd);
};