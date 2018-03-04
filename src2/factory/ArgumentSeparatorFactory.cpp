#include "ArgumentSeparatorFactory.h"

// TODO: add new application headers
#include "example/cc.h"

using namespace std;

using parent_t = ArgumentSeparatorFactory::parent_t;

template <>
const std::string parent_t::optName("separator");
template <>
const std::string parent_t::usagePrefix(
	"Use the following parameters to select ONE operation\n");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void ArgumentSeparatorFactory::init()
{
	// TODO: add new strategy here
	registerClass<ConnectedComponent::separator_t>(ConnectedComponent::name);
	
}
