#include "SharderFactory.h"

using namespace std;

using parent_t = SharderFactory::parent_t;

template <>
const std::string parent_t::optName("sharder");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void SharderFactory::init()
{
	// TODO: add new strategy here
	registerClass<SharderMod>("mod", "mod. Use mod method to spread nodes.");
	
}
