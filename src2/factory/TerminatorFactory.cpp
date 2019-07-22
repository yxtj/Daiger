#include "TerminatorFactory.h"

using namespace std;

using parent_t = TerminatorFactory::parent_t;

template <>
const std::string parent_t::optName("terminator");
template <>
const std::string parent_t::usagePrefix("");

template <>
std::map<std::string, parent_t::createFun> parent_t::contGen{};
template <>
std::map<std::string, std::string> parent_t::contUsage{};


void TerminatorFactory::init()
{
	// TODO: add new strategy here
	registerClass<TerminatorStop>("stop", "stop. Terminate when there is no change.");
	registerClass<TerminatorDiffValue>("diff", "diff <v>."
		" Terminate when the difference of progress values in consecutive reports is smaller than <v>.");
	registerClass<TerminatorDiffRatio>("diffr", "diffr <r>."
		" Terminate when the relative difference of progress values in consecutive reports is smaller than <r>.");
	registerClass<TerminatorVariance>("var", "var <r>."
		" Terminate when the recent variance of progress values bump within <r>.");
}
