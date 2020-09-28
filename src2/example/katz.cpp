#include "katz.h"
#include "util/Util.h"
#include "common/def_func.h"
#include <vector>
#include <algorithm>

using namespace std;

const std::string Katz::name("katz");

std::string Katz::getName() const {
	return name;
}

void Katz::reg(){
	AppKernelFactory::registerClass<Katz>(name);
}

ArgumentSeparator* Katz::generateSeparator(){
	return new separator_t();
}
OperationBase* Katz::generateOperation(){
	return new operation_t();
}
IOHandlerBase* Katz::generateIOHandler(){
	return new iohandler_t();
}
ProgressorBase* Katz::generateProgressor(){
	return new progressor_t();
}
GlobalHolderBase* Katz::generateGraph(){
	return new graph_t();
}

// -------- Components --------

void Katz::MyOperation::init(const std::vector<std::string>& arg_line, const size_t nInstance){
	source = stoid(arg_line[0]); 
	beta = stod(arg_line[1]);
	use_degree = beTrueOption(arg_line[2]);
	dummy_id = gen_dummy_id(1);
	if(!use_degree)
		pfun = &Katz::MyOperation::priority_v;
	else
		pfun = &Katz::MyOperation::priority_vn;
}
Katz::MyOperation::node_t Katz::MyOperation::preprocess_node(
	const id_t& k, neighbor_list_t& neighbors)
{
	// remove self-loop
	auto it = find_if(neighbors.begin(), neighbors.end(), [=](const neighbor_t& n){
		return n == k;
	});
	if(it!=neighbors.end())
		neighbors.erase(it);
	return make_node(k, k == source ? 1.0 : 0.0, neighbors);
}
std::vector<Katz::MyOperation::DummyNode> Katz::MyOperation::dummy_nodes(){
	DummyNode res;
	neighbor_list_t onb;
	onb.push_back(source);
	res.node = make_node(dummy_id, 1.0, onb);
	res.type = DummyNodeType::NORMAL;
	res.func = [=](const id_t& id){
		return make_pair(id != dummy_id, id);
	};
	return { res };
}
bool Katz::MyOperation::is_dummy_node(const id_t& id){
	return id == dummy_id;
}
Katz::value_t Katz::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
    return beta * n.v;
}
// scheduling - priority
priority_t Katz::MyOperation::priority(const node_t& n){
	return (*pfun)(n);
}
priority_t Katz::MyOperation::priority_v(const node_t& n){
	value_t p = abs(n.u - n.v);
	return static_cast<priority_t>(p);
}
priority_t Katz::MyOperation::priority_vn(const node_t& n){
	value_t p = abs(n.u - n.v);
	return static_cast<priority_t>(p*n.onb.size());
}

// <source> <beta> <use-degree-priority>
AppArguments Katz::MySeparator::separate(const std::vector<std::string>& args){
	if(args.size() < 1 || args.size() > 2){
		throw invalid_argument("Katz Parameter: <source> <beta> [degree-priority]. degree-priority=true");
	}
	AppArguments res;
	res.name = Katz::name;
	if(args.size() == 1)
		res.operation_arg = { args[0], args[1], "true" };
	else
		res.operation_arg = { args[0], args[1], args[2] };
	res.iohandler_arg = {};
	res.progressor_arg = {};
	return res;
}
