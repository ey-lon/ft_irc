#include <map>
#include <iostream>

class User;

class Channel
{
	private:
		std::string							_name;
		std::string							_password;
		std::string							_topic;
		std::string							_mode;

		std::map<std::string, bool>			_users;
		//std::vector<std::string>			_bannedUsers;

		int									_usersLimit;

		Channel(void);

	public:
		Channel(const std::string & channelName);
		~Channel(void);
		
		//getters
		const std::map <std::string, bool> &	getUsers(void) const;
		const std::string	getName(void) const;
		const std::string	getPassword(void) const;
		const std::string	getTopic(void) const;
		const std::string	getMode(void) const;
		int					getUsersLimit(void) const;
		int					nUsers(void) const;
		bool				hasFlag(char flag) const;

		//setters
		void				setName(const std::string & name);
		void				setPassword(const std::string & password);
		void				setTopic(const std::string & topic);
		void				setUsersLimit(int);
		void				addMode(const std::string & mode);
		void				addMode(char flag);
		void				removeMode(const std::string & mode);
		void				removeMode(char flag);

		//users
		bool				isUserPresent(const std::string & nickName) const;
		bool				isUserOperator(const std::string & nickName) const;
		
		void				addUser(const std::string & nickname);
		void				updateNick(const std::string & oldNick, const std::string & newNick);
		void				removeUser(const std::string & nickName);
		void				promoteUser(const std::string & nickName);
		void				demoteUser(const std::string & nickName);
};
