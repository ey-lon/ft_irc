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
std::vector<std::string> splitString(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        result.push_back(token);
    }

    return result;
}