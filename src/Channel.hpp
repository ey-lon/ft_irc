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
		bool								_isInviteOnly;
		std::map <std::string, t_user *>	_users;

	public:
		Channel(void);
		~Channel(void);

		Channel(const std::string & channelName);
		
		//getters
		const std::string	getName(void) const;
		bool				isInviteOnly(void) const;

		//setters
		void				setName(const std::string & name);
		void				setInviteOnly(bool isInviteOnly);

		//users
		bool				isUserPresent(const std::string & userName) const;
		bool				isUserOperator(const std::string & userName) const;
		
		void				addUser(User *user);
		void				removeUser(const std::string & userName);
		void				promoteUser(const std::string & userName);
		void				demoteUser(const std::string & userName);
};
