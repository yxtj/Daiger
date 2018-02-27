#include "ApplicationFactory.h"

// TODO: add new application headers
#include "cc.h"

using namespace std;

using parent_t = ApplicationFactory::parent_t;

template <>
const std::string parent_t::optName("strategy");
template <>
const std::string parent_t::usagePrefix(
	"Use the following parameters to select ONE learning strategy\n");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void ApplicationFactory::init()
{
	// TODO: add new strategy here
	registerInOne<ConnectedComponent>();
}

KernelBase * ApplicationFactory::generate(const std::string & name)
{
	FactoryProductTemplate* p = parent_t::generate(name);
	KernelBase* res = dynamic_cast<KernelBase*>(p);
	return res;
}
