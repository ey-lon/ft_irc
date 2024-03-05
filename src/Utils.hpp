#include <iostream>
#include <vector>

std::string					strTrim		(const std::string &);
std::vector<std::string>	splitString	(const std::string &);
std::vector<std::string>	splitString	(const std::string &, char);
std::vector<std::string>	parseInput	(const std::string &);

bool						isCorrectPassword(const std::string & password);
bool						isCorrectName(const std::string & name);