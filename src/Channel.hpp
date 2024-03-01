#include <map>
#include <iostream>

class User;

class Channel
{
	typedef struct s_user
	{
		User *	userPtr;
		bool	isOperator;
	} t_user;

	private:
		std::string							_name;
		std::string							_password;
		std::string							_topic;
		bool								_isInviteOnly;
		std::map <std::string, t_user *>	_users;

		Channel(void);

	public:
		Channel(const std::string & channelName);
		~Channel(void);
		
		//getters
		const std::string	getName(void) const;
		const std::string	getPassword(void) const;
		const std::string	getTopic(void) const;
		bool				isInviteOnly(void) const;

		//setters
		void				setName(const std::string & name);
		void				setPassword(const std::string & password);
		void				setTopic(const std::string & topic);
		void				setInviteOnly(bool isInviteOnly);

		//users
		bool				isUserPresent(const std::string & userName) const;
		bool				isUserOperator(const std::string & userName) const;
		
		void				addUser(User *user);
		void				removeUser(const std::string & userName);
		void				promoteUser(const std::string & userName);
		void				demoteUser(const std::string & userName);
};
