#include <map>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "Errors.hpp"

class Channel;

class User;

class Server
{
	private:
		Server(void);
		const std::string					_serverName;
		int									_serverSocket;
		sockaddr_in							_serverAddress;
		char								_hostname[256];

		char *								_ip;
		u_int16_t							_port;
		std::string							_password;
		bool								_isRunning;

		std::map<std::string, Channel *>	_channels;
		std::map<int, User *>				_users;
		std::vector<pollfd>					_fds;

		Errors								_errors;

		//communication
		void				channelMegaphone(Channel * channel, User * user, const std::string & msg) const;
		void				serverMegaphone	(User * user, const std::string & msg) const;
		void				joinMsg			(Channel * channel, User * user);
		void				welcomeMsg		(User * user);
		void				errorMsg		(User * user, int code);
		void				dealCommand		(std::vector<std::string> argv, User * user);
		int					dealMessage		(int userFd);

		//commands
		void				pass			(std::vector<std::string> argv, User * user);
		void				user			(std::vector<std::string> argv, User * user);
		void				nick			(std::vector<std::string> argv, User * user);
		void				join			(std::vector<std::string> argv, User * user);
		void				privmsg			(std::vector<std::string> argv, User * user);
		void				invite			(std::vector<std::string> argv, User * user);
		void				kick			(std::vector<std::string> argv, User * user);
		void				topic			(std::vector<std::string> argv, User * user);
		void				mode			(std::vector<std::string> argv, User * user);
		void				part			(std::vector<std::string> argv, User * user);
		void				quit			(std::vector<std::string> argv, User * user);
		void				ping			(std::vector<std::string> argv, User * user);

	public:
		Server(int port, const std::string & password);					//constructor where port is given as an int
		Server(const std::string & port, const std::string & password); //constructor where port is given as a literal int
		~Server(void);

		void				init(void);
		void				run (void);
		void				stop(void);

		//getters
		const std::string &	getName		(void) const;
		const std::string &	getPassword	(void) const;
		int					getPort		(void) const;
		int					nUsers		(void) const;
		int					nChannels	(void) const;
		bool				isRunning	(void) const;

		//setters
		void				setPassword	(const std::string & password);

		//channels
		Channel *			getChannelByName	(const std::string & channelName) const;
		Channel *			createChannel		(const std::string & channelName);
		void				removeChannel		(const std::string & channelName);

		//users
		User *				getUserByFd			(int userFd) const;
		User *				getUserByUserName	(const std::string & userName) const;
		User *				getUserByNickName	(const std::string & nickName) const;
		User *				createUser			(void);
		void				removeUser			(int userFd);
};
