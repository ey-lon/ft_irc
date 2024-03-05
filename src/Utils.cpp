#include "Utils.hpp"

std::string strTrim(const std::string & s) {
	std::string str = s;
	size_t i = 0;
	while (std::isspace(str[i])) {
		i++;
	}
	if (i == str.length()) {
		str.clear();
		return (str);
	}
	size_t j = str.length() - 1;
	while (j >= 0 && std::isspace(str[j])) {
		j--;
	}
	str = str.substr(i, j - i + 1);
	return (str);
}

#include <sstream>
std::vector<std::string>	splitString(const std::string & input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

std::vector<std::string>	splitString(const std::string & input, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        result.push_back(strTrim(token));
    }
	return (result);
}

std::vector<std::string>	parseInput(const std::string & input) {
	std::string	str = input;
	size_t colonPos = str.find(':');
	std::string	spacedString;

	if (colonPos != std::string::npos) {
		spacedString = strTrim(str.substr(colonPos + 1));
		str = str.substr(0, colonPos);
	}
	std::vector<std::string> vec = splitString(str);
	if (!spacedString.empty()) {
		vec.push_back(spacedString);
	}
	return (vec);
}

bool	isCorrectName(const std::string & name) {
	for (size_t i = 0; i < name.length(); i++) {
		if (!std::isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
			return (false);
		}
	}
	return (true);
}

bool	isCorrectPassword(const std::string & password) {
	if (password.empty()) {
		return (false);
	}
	else if (std::isspace(password[0]) || std::isspace(password[password.length() - 1])) {
		return (false);
	}
	else {
		return (true);
	}
}