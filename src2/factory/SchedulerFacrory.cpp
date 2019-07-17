#include "SchedulerFactory.h"

using namespace std;

using parent_t = SchedulerFactory::parent_t;

template <>
const std::string parent_t::optName("scheduler");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void SchedulerFactory::init()
{
	// TODO: add new strategy here
	registerClass<SchedulerRoundRobin>("rr",
		"rr. Round-Robin for all nodes.");
	registerClass<SchedulerPriorityMaintain>("priorityM",
		"priorityM <portion>. Pick the top <portion> percent local nodes. Maintain top-k for each update.");
	registerClass<SchedulerPrioritySelection>("priority",
		"priority <portion>. Pick the top <portion> percent local nodes. Only do one selection when it is needed");
	registerClass<SchedulerFIFO>("fifo",
		"fifo. Run on all affected nodes, according to the order they got touched.");
	
}
