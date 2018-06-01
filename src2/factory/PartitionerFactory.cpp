#include "PartitionerFactory.h"

using namespace std;

using parent_t = PartitionerFactory::parent_t;

template <>
const std::string parent_t::optName("partitioner");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void PartitionerFactory::init()
{
	// TODO: add new strategy here
	registerClass<PartitionerMod>("mod", "mod. Use mod method to spread nodes.");
	
}
