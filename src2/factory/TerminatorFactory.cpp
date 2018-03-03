#include "TerminatorFactory.h"

// TODO: add new application headers
#include "example/cc.h"

using namespace std;

using parent_t = TerminatorFactory::parent_t;

template <>
const std::string parent_t::optName("terminator");
template <>
const std::string parent_t::usagePrefix(
	"Use the following parameters to select ONE operation\n");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void TerminatorFactory::init()
{
	// TODO: add new strategy here
	registerClass<ConnectedComponent::teminator_t>(ConnectedComponent::name);
	
}
