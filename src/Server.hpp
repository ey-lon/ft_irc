#include <map>
#include <iostream>
#include <netinet/in.h>

class Channel;

class Client;

class Server
{
	private:
		std::map<std::string, Channel *>	_channels;
		std::map<std::string, Client *>		_clients;
		std::string							_password;
		int									_port;

		int									_serverSocket;
		sockaddr_in							_serverAddress;

	public:
		Server(void);
		~Server(void);

		Server(int port, const std::string & password);

		void				start(void);
		void				loop(void);

		void				newConnection(void);
		void				dealMessage(void);

		//getters
		const std::string &	getPassword(void) const;
		int					getPort(void) const;
		int					nClients(void) const;
		int					nChannels(void) const;

		//setters
		void				setPassword(const std::string & password);

		//channels
		void				createChannel(const std::string & channelName);
		void				addChannel(Channel * channel);
		void				removeChannel(const std::string & channelName);

		//clients
		void				createClient(int fd, const std::string & clientName);
		void				addClient(Client * client);
		void				removeClient(const std::string & clientName);
};