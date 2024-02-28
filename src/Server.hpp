#include <map>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>

class Channel;

class User;

class Server
{
	private:
		int									_serverSocket;
		sockaddr_in							_serverAddress;
		char								_hostname[256];

		int									_port;
		std::string							_password;

		std::map<std::string, Channel *>	_channels;
		std::map<int, User *>				_users;
		std::vector<pollfd>					_fds;

	public:
		Server(void); // <-- only for testing (to be removed)
		Server(int port, const std::string & password);

		~Server(void);

		void				start(void);
		void				loop(void);

		void				newConnection(void);
		int					dealMessage(int);

		void				welcome(User * user);

		//getters
		const std::string &	getPassword(void) const;
		int					getPort(void) const;
		int					nUsers(void) const;
		int					nChannels(void) const;

		//setters
		void				setPassword(const std::string & password);

		//channels
		Channel *			getChannelByName(const std::string & channelName) const;
		Channel *			createChannel(const std::string & channelName);
		void				removeChannel(const std::string & channelName);

		//users
		User *				getUserByFd(int userFd) const;
		User *				getUserByName(const std::string & userName) const;
		User *				createUser(int fd);
		void				removeUser(int userFd);
};