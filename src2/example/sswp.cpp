#include "sswp.h"
#include "util/Util.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace std;

const std::string WidestPath::name("sssp");

std::string WidestPath::getName() const {
	return name;
}

void WidestPath::reg(){
	AppKernelFactory::registerClass<WidestPath>(name);
}

ArgumentSeparator* WidestPath::generateSeparator(){
	return new separator_t();
}
OperationBase* WidestPath::generateOperation(){
	return new operation_t();
}
IOHandlerBase* WidestPath::generateIOHandler(){
	return new iohandler_t();
}
TerminatorBase* WidestPath::generateTerminator(){
	return new terminator_t();
}
GlobalHolderBase* WidestPath::generateGraph(){
	return new graph_t();
}

// -------- Components --------

void WidestPath::MyOperation::init(const std::vector<std::string>& arg_line){
	source = stoid(arg_line[0]);
	use_degree = beTrueOption(arg_line[1]);
}
WidestPath::MyOperation::node_t WidestPath::MyOperation::preprocess_node(
	const id_t& k, neighbor_list_t& neighbors)
{
	if(k == source)
		neighbors.emplace_back(k, numeric_limits<value_t>::infinity()); // add an dummy self loop for the incremental case
	return make_node(k, k == source ? numeric_limits<value_t>::infinity() : identity_element(), neighbors);
}
WidestPath::value_t WidestPath::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return min(n.v, neighbor.second);
}
// scheduling - priority
priority_t WidestPath::MyOperation::priority(const node_t& n){
	double p = n.u;
	return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <source> <use-degree-priority> <epsilon-termination>
AppArguments WidestPath::MySeparator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = WidestPath::name;
	res.operation_arg = {args[0], args[1]};
	res.iohandler_arg = {};
	res.terminator_arg = {args[2]};
	return res;
}
