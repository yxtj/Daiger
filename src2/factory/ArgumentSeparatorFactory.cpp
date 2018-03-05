#include "ArgumentSeparatorFactory.h"

using namespace std;

using parent_t = ArgumentSeparatorFactory::parent_t;

template <>
const std::string parent_t::optName("separator");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void ArgumentSeparatorFactory::init()
{
	// application classes are registered in their implementations
}
