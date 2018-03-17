#include "pr.h"
#include <vector>
#include <algorithm>

using namespace std;

const std::string PageRank::name("pr");

std::string PageRank::getName() const {
	return name;
}

void PageRank::reg(){
	AppKernelFactory::registerClass<PageRank>(name);
}

ArgumentSeparator* PageRank::generateSeparator(){
	return new separator_t();
}
OperationBase* PageRank::generateOperation(){
	return new operation_t();
}
IOHandlerBase* PageRank::generateIOHandler(){
	return new iohandler_t();
}
TerminatorBase* PageRank::generateTerminator(){
	return new terminator_t();
}
GlobalHolderBase* PageRank::generateGraph(){
	return new graph_t();
}

// -------- Components --------

void PageRank::MyOperation::init(const std::vector<std::string>& arg_line){
	damp = stod(arg_line[0]);
	if(damp < 0.0 || damp > 1.0){
		throw invalid_argument("Invalid damping factor: "+arg_line[0]);
	}
	vector<string> true_options({"1", "t", "T", "true", "True", "TRUE", "y", "Y", "yes", "Yes", "YES"});
	use_degree = find(true_options.begin(), true_options.end(), arg_line[1]) != true_options.end();
}
PageRank::value_t PageRank::MyOperation::init_value(const id_t& k, const neighbor_list_t& neighbors){
	return 1.0 - damp;
}
PageRank::value_t PageRank::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return damp * n.v / n.onb.size();
}
// scheduling - priority
priority_t PageRank::MyOperation::priority(const node_t& n){
	double p = n.u - n.v;
	return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <damp-factor> <use-degree-priority> <epsilon-termination>
AppArguments PageRank::MySeparator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = PageRank::name;
	res.operation_arg = {args[0], args[1]};
	res.iohandler_arg = {};
	res.terminator_arg = {args[2]};
	return res;
}
