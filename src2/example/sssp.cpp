#include "sssp.h"
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

const std::string ShortestPath::name("sssp");

std::string ShortestPath::getName() const {
	return name;
}

void ShortestPath::reg(){
	AppKernelFactory::registerClass<ShortestPath>(name);
}

ArgumentSeparator* ShortestPath::generateSeparator(){
	return new separator_t();
}
OperationBase* ShortestPath::generateOperation(){
	return new operation_t();
}
IOHandlerBase* ShortestPath::generateIOHandler(){
	return new iohandler_t();
}
TerminatorBase* ShortestPath::generateTerminator(){
	return new terminator_t();
}
GlobalHolderBase* ShortestPath::generateGraph(){
	return new graph_t();
}

// -------- Components --------

void ShortestPath::MyOperation::init(const std::vector<std::string>& arg_line){
	source = stoid(arg_line[0]);
	vector<string> true_options({"1", "t", "T", "true", "True", "TRUE", "y", "Y", "yes", "Yes", "YES"});
	use_degree = find(true_options.begin(), true_options.end(), arg_line[1]) != true_options.end();
}
ShortestPath::value_t ShortestPath::MyOperation::init_value(const id_t& k, const neighbor_list_t& neighbors){
	return k == source ? 0 : numeric_limits<double>::max();
}
ShortestPath::value_t ShortestPath::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v + neighbor.second;
}
// scheduling - priority
priority_t ShortestPath::MyOperation::priority(const node_t& n){
	double p = n.u;
	return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <source> <use-degree-priority> <epsilon-termination>
AppArguments ShortestPath::MySeparator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = ShortestPath::name;
	res.operation_arg = {args[0], args[1]};
	res.iohandler_arg = {};
	res.terminator_arg = {args[2]};
	return res;
}
