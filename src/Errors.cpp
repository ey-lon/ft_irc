#include "Errors.hpp"

Errors::Errors(void) : _unknownErr("unknown error") {
	_errors[400] = "Invalid input";
	_errors[401] = "No such nickname";
	_errors[402] = "No such server";
	_errors[403] = "No such channel";
	_errors[404] = "Cannot send to channel";
	_errors[405] = "You have joined too many channels";
	_errors[406] = "There was no such nickname";
	_errors[407] = "Too many recipients, can't send message";
	_errors[409] = "No origin specified";
	_errors[411] = "No recipient given";
	_errors[412] = "No text to send";
	_errors[413] = "No toplevel domain specified";
	_errors[414] = "Wildcard in toplevel domain";
	_errors[421] = "Unknown command";
	_errors[422] = "MOTD file is missing";
	_errors[431] = "No nickname given";
	_errors[432] = "Erroneous nickname";
	_errors[433] = "Nickname is already in use";
	_errors[436] = "Nickname collision KILL";
	_errors[441] = "They aren't on that channel";
	_errors[442] = "You're not on that channel";
	_errors[443] = "User is already on that channel";
	_errors[444] = "User not logged in";
	_errors[445] = "SUMMON has been disabled";
	_errors[446] = "USERS has been disabled";
	_errors[451] = "You have not registered";
	_errors[461] = "Not enough parameters";
	_errors[462] = "You may not reregister";
	_errors[463] = "Your host isn't among the privileged";
	_errors[464] = "Password incorrect and/or required";
	_errors[465] = "You are banned from this server";
	_errors[466] = "You will be banned";
	_errors[467] = "Channel key already set";
	_errors[468] = "Invalid username";
	_errors[471] = "Cannot join/invite channel (+l)";
	_errors[472] = "Unknown mode char";
	_errors[473] = "Cannot join channel (+i)";
	_errors[474] = "Cannot join channel (+b)";
	_errors[475] = "Cannot join channel (+k)";
	_errors[481] = "Permission Denied- You're not an IRC operator";
	_errors[482] = "You're not channel operator";
	_errors[483] = "You can't kill a server!";
	_errors[484] = "Your connection is restricted!";
	_errors[485] = "You're not the original channel operator";
	_errors[491] = "No O-lines for your host";
	_errors[501] = "Unknown MODE flag";
	_errors[502] = "Can't change mode for other users";
}

Errors::~Errors(void) {}

const std::string &Errors::operator[](int index) const {
	std::map<int, std::string>::const_iterator it = _errors.find(index); 
	if (it != _errors.end()) {
		return (it->second);
	}
	else {
		return (_unknownErr);
	}
}