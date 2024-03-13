#include <map>
#include <iostream>

class Errors {
	private:
		std::map<int, std::string>	_errors;
		const std::string			_unknownErr;

	public:
		Errors();
		~Errors();

		const std::string &operator[](int index) const;
};