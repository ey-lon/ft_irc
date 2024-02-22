#include <iostream>
#include <map>

class Channel;

class Client
{
	private:
		int									_fd;
		std::string							_name;
		std::map<std::string, Channel *>	_channels;

	public:
		Client(void);
		~Client(void);

		Client(int fd, const std::string & clientName);

		//getters
		int					getFd() const;
		const std::string &	getName() const;

		bool				isClientInChannel(std::string channelName) const;
		bool				isClientAdmin(std::string channelName) const;

		//
		void				addChannel(Channel * channel);
		void				removeChannel(const std::string & channelName);
};
