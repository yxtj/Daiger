#include "mc.h"
#include "util/Util.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace std;

const std::string MarkovChain::name("mc");

std::string MarkovChain::getName() const {
	return name;
}

void MarkovChain::reg(){
	AppKernelFactory::registerClass<MarkovChain>(name);
}

ArgumentSeparator* MarkovChain::generateSeparator(){
	return new separator_t();
}
OperationBase* MarkovChain::generateOperation(){
	return new operation_t();
}
IOHandlerBase* MarkovChain::generateIOHandler(){
	return new iohandler_t();
}
TerminatorBase* MarkovChain::generateTerminator(){
	return new terminator_t();
}
GlobalHolderBase* MarkovChain::generateGraph(){
	return new graph_t();
}

// -------- Components --------

void MarkovChain::MyOperation::init(const std::vector<std::string>& arg_line){
	use_degree = beTrueOption(arg_line[0]);
}
MarkovChain::MyOperation::node_t MarkovChain::MyOperation::preprocess_node(
	const id_t& k, neighbor_list_t& neighbors)
{
	return make_node(k, 1.0, neighbors);
}
MarkovChain::value_t MarkovChain::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v * neighbor.second;
}
// scheduling - priority
priority_t MarkovChain::MyOperation::priority(const node_t& n){
	double p = abs<double>(n.v - n.u);
	return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <source> <use-degree-priority> <epsilon-termination>
AppArguments MarkovChain::MySeparator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = MarkovChain::name;
	res.operation_arg = {args[0]};
	res.iohandler_arg = {};
	res.terminator_arg = {args[1]};
	return res;
}
