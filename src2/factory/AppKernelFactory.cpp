#include "AppKernelFactory.h"

using namespace std;

using parent_t = AppKernelFactory::parent_t;

template <>
const std::string parent_t::optName("app");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void AppKernelFactory::init()
{
	// application classes are registered in their implementations
}
