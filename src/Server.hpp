#include <map>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

class Channel;

class User;

class Server
{
	private:
		const std::string					_serverName;
		int									_serverSocket;
		sockaddr_in							_serverAddress;
		char								_hostname[256];

		char *								_ip;
		u_int16_t							_port;
		std::string							_password;

		std::map<std::string, Channel *>	_channels;
		std::map<int, User *>				_users;
		std::vector<pollfd>					_fds;

		//communication
		void				joinMsg			(Channel *channel, User *user);
		void				welcomeMsg		(User * user);
		void				authorization	(std::vector<std::string> argv, User * user);
		void				login			(std::vector<std::string> argv, User * user);
		void				dealCommand		(std::vector<std::string> argv, User * user);
		int					dealMessage		(int userFd);
		void				newConnection	(void);

		//commands
		void				cap				(std::vector<std::string> argv, User * user);
		void				pass			(std::vector<std::string> argv, User * user);
		void				user			(std::vector<std::string> argv, User * user);
		void				nick			(std::vector<std::string> argv, User * user);
		void				join			(std::vector<std::string> argv, User * user);
		void				privmsg			(std::vector<std::string> argv, User * user);
		void				invite			(std::vector<std::string> argv, User * user);
		void				kick			(std::vector<std::string> argv, User * user);
		void				topic			(std::vector<std::string> argv, User * user);
		void				mode			(std::vector<std::string> argv, User * user);
		void				ping			(std::vector<std::string> argv, User * user);

	public:
		Server(void); // <-- only for testing (to be removed)
		Server(int port, const std::string & password);
		Server(const std::string & port, const std::string & password);
		~Server(void);

		void				start(void);
		void				loop(void);

		//getters
		const std::string &	getName(void) const;
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
		User *				getUserByUserName(const std::string & userName) const;
		User *				getUserByNickName(const std::string & nickName) const;
		User *				createUser(int fd);
		void				removeUser(int userFd);
};
