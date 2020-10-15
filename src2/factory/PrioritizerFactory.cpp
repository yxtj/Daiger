#include "PrioritizerFactory.h"
#include <stdexcept>

using namespace std;

using parent_t = PrioritizerFactory::parent_t;

template <>
const std::string parent_t::optName("prioritizer");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};

PrioritizerBase* PrioritizerFactory::generate(const std::string& name){
	throw logic_error("This generate function should not be called. Call the typed version instead.");
	return nullptr;
}

void PrioritizerFactory::init()
{
	// application classes are registered in their implementations
	registerDummy("none", "none. Do not use priority (always return 0.0).");
	//registerClass<PriorityValue>("value", "value. Use the current value as the priority.");
	//registerClass<PriorityValueODeg>("value_deg", "value_deg. Use the current value * out-degree as the priority.");
	//registerClass<PriorityDiff>("diff", "diff. Use the difference between current value and old value as the priority.");
	//registerClass<PriorityDiffODeg>("diff_deg", "diff_deg. Use the value difference * out-degree as the priority.");
	registerDummy("value", "value. Use the current value as the priority.");
	registerDummy("value_deg", "value_deg. Use the current value * out-degree as the priority.");
	registerDummy("diff", "diff. Use the difference between current value and old value as the priority.");
	registerDummy("diff_deg", "diff_deg. Use the value difference * out-degree as the priority.");
}
