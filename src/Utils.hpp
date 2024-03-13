#include <iostream>
#include <vector>

std::string					toString	(int);
std::string					strTrim		(const std::string &);
std::vector<std::string>	splitString	(const std::string &);
std::vector<std::string>	splitString	(const std::string &, char);
std::vector<std::string>	parseInput	(const std::string &);

bool						isValidName(const std::string & name);
bool						isValidPassword(const std::string & password);
bool						isValidPort(const std::string & port);