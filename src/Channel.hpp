#include <map>
#include <iostream>

class User;

class Channel
{
	typedef struct s_user
	{
		User *	user;
		bool	isOperator;
	} t_user;

	private:
		std::string							_name;
		bool								_isInviteOnly;
		std::map <std::string, t_user *>	_users;

	public:
		Channel(void);
		~Channel(void);

		Channel(const std::string & channelName);
		
		//getters
		const std::string	getName(void) const;
		bool				isInviteOnly(void) const;

		bool				isUserPresent(const std::string & userUserName) const;
		bool				isUserOperator(const std::string & userUserName) const;

		//
		void				addUser(User *user);
		void				removeUser(const std::string & userUserName);
		void				promoteUser(const std::string & userUserName);
		void				demoteUser(const std::string & userUserName);
};
