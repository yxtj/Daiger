#include "cc.h"
#include <string>
#include <numeric>
//#include <algorithm>

using namespace std;

const std::string ConnectedComponent::name("cc");

std::string ConnectedComponent::getName() const {
	return name;
}

void ConnectedComponent::reg(){
	AppKernelFactory::registerClass<ConnectedComponent>(name);
	
	// ArgumentSeparatorFactory::registerClass<separator_t>(name);
	// OperationFactory::registerClass<operation_t>(name);
	// IOHandlerFactory::registerClass<iohandler_t>(name);
	// TerminatorFactory::registerClass<terminator_t>(name);
}

ArgumentSeparator* ConnectedComponent::generateSeparator(){
	return new separator_t();
}
OperationBase* ConnectedComponent::generateOperation(){
	return new operation_t();
}
IOHandlerBase* ConnectedComponent::generateIOHandler(){
	return new iohandler_t();
}
TerminatorBase* ConnectedComponent::generateTerminator(){
	return new terminator_t();
}
GlobalHolderBase* ConnectedComponent::generateGraph(){
	return new graph_t();
}

// -------- Components --------

void ConnectedComponent::MyOperation::init(const std::vector<std::string>& arg_line){
}
ConnectedComponent::MyOperation::node_t ConnectedComponent::MyOperation::preprocess_node(
	const id_t& k, neighbor_list_t& neighbors)
{
	return make_node(k, k, neighbors);
}
ConnectedComponent::value_t ConnectedComponent::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v;
}
// scheduling - priority
priority_t ConnectedComponent::MyOperation::priority(const node_t& n){
	return static_cast<priority_t>(n.v);
}

AppArguments ConnectedComponent::MySeparator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = ConnectedComponent::name;
	res.operation_arg = {};
	res.iohandler_arg = {};
	res.terminator_arg = {};
	return res;
}
