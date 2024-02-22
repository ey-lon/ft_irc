

#include <map>
#include <iostream>

class Client;

class Channel
{
	typedef struct s_client
	{
		Client *	client;
		bool		isAdmin;
	} t_client;

	private:
		std::string							_name;
		bool								_isInviteOnly;
		std::map <std::string, t_client *>	_clients;

	public:
		Channel(void);
		~Channel(void);

		Channel(const std::string & channelName);

		//getters
		const std::string	getName(void) const;
		bool				isInviteOnly(void) const;

		bool				isClientInChannel(const std::string & clientName) const;
		bool				isClientAdmin(const std::string & clientName) const;

		//
		void				addClient(Client *client);
		void				removeClient(const std::string & clientName);
		void				promoteClient(const std::string & clientName);
		void				demoteClient(const std::string & clientName);
};

