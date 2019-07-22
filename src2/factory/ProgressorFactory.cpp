#include "ProgressorFactory.h"

using namespace std;

using parent_t = ProgressorFactory::parent_t;

template <>
const std::string parent_t::optName("progressor");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void ProgressorFactory::init()
{
	// application classes are registered in their implementations
}
