#pragma once
#include <string>
#include <vector>

struct AppArguments {
	std::string name;
	std::vector<std::string> operation_arg;
	std::vector<std::string> iohandler_arg;
	std::vector<std::string> terminator_arg;
};

class ArgumentSeparator {
public:
	virtual ~ArgumentSeparator() = default;
	virtual AppArguments separate(const std::vector<std::string>& args, const size_t nParts) = 0;
};
