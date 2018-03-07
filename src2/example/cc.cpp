#include "cc.h"
#include <string>
#include <numeric>
//#include <algorithm>

using namespace std;

const std::string ConnectedComponent::name("cc");

ConnectedComponent::ConnectedComponent(){
	OperationFactory::registerClass<operation_t>(name);
	IOHandlerFactory::registerClass<iohandler_t>(name);
	TerminatorFactory::registerClass<terminator_t>(name);
	ArgumentSeparatorFactory::registerClass<separator_t>(name);
}

void ConnectedComponent::MyOperation::init(const std::vector<std::string>& arg_line){
}

ConnectedComponent::value_t ConnectedComponent::MyOperation::identity_element() const{
	return std::numeric_limits<value_t>::min();
}
ConnectedComponent::value_t ConnectedComponent::MyOperation::oplus(const value_t& a, const value_t& b){
	//return max(a, b);
	return (a<b)?b:a;
}
ConnectedComponent::value_t ConnectedComponent::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v;
}
bool ConnectedComponent::MyOperation::better(const value_t& a, const value_t& b){
	return a > b;
}
// scheduling - priority
priority_t ConnectedComponent::MyOperation::priority(const node_t& n){
	return static_cast<priority_t>(n.v);
}

AppArguments ConnectedComponent::Separator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = ConnectedComponent::name;
	res.operation_arg = {};
	res.iohandler_arg = {};
	res.terminator_arg = {};
	return res;
}
