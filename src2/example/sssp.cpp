#include "sssp.h"
#include "util/Util.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

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
ProgressorBase* ShortestPath::generateProgressor(){
	return new progressor_t();
}
GlobalHolderBase* ShortestPath::generateGraph(){
	return new graph_t();
}
PrioritizerBase* ShortestPath::generatePrioritizer(const std::string& name){
	return PrioritizerFactory::generate<value_t, neighbor_t>(name);
}

// -------- Components --------

void ShortestPath::MyOperation::init(const std::vector<std::string>& arg_line, const size_t nInstance){
	source = stoid(arg_line[0]);
	use_degree = beTrueOption(arg_line[1]);
}
ShortestPath::MyOperation::node_t ShortestPath::MyOperation::preprocess_node(
	const id_t& k, neighbor_list_t& neighbors)
{
	if(k == source)
		neighbors.emplace_back(k, 0.0); // add an dummy self loop for the incremental case
	return make_node(k, k == source ? 0.0 : identity_element(), neighbors);
}
ShortestPath::value_t ShortestPath::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v + neighbor.second;
}
// scheduling - priority
priority_t ShortestPath::MyOperation::priority(const node_t& n){
	double p = -n.u; // the smaller the better
	return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <source> <use-degree-priority>
AppArguments ShortestPath::MySeparator::separate(const std::vector<std::string>& args){
	if(args.size() < 1 || args.size() > 2){
		throw invalid_argument("SSSP Parameter: <source> [degree-priority]. degree-priority=false");
	}
	AppArguments res;
	res.name = ShortestPath::name;
	if(args.size() == 1)
		res.operation_arg = { args[0], "false" };
	else
		res.operation_arg = { args[0], args[1] };
	res.iohandler_arg = {};
	res.progressor_arg = {};
	return res;
}
