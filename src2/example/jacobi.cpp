#include "jacobi.h"
#include "util/Util.h"
#include <algorithm>
#include <vector>

using namespace std;

const std::string Jacobi::name("jacobi");

std::string Jacobi::getName() const { return name; }

void Jacobi::reg() { AppKernelFactory::registerClass<Jacobi>(name); }

ArgumentSeparator* Jacobi::generateSeparator() { return new separator_t(); }
OperationBase* Jacobi::generateOperation() { return new operation_t(); }
IOHandlerBase* Jacobi::generateIOHandler() { return new iohandler_t(); }
ProgressorBase* Jacobi::generateProgressor() { return new progressor_t(); }
GlobalHolderBase* Jacobi::generateGraph() { return new graph_t(); }
PrioritizerBase* Jacobi::generatePrioritizer(const std::string& name){
    return PrioritizerFactory::generate<value_t, neighbor_t>(name);
}
// -------- Components --------

void Jacobi::MyOperation::init(const std::vector<std::string>& arg_line) {
	constant_id = stoid(arg_line[0]);
    use_degree = beTrueOption(arg_line[1]);
}
Jacobi::MyOperation::node_t Jacobi::MyOperation::preprocess_node(
    const id_t& k, neighbor_list_t& neighbors) {
    return make_node(k, k == constant_id? 1.0 : 0.0, neighbors);
}
std::vector<Jacobi::MyOperation::DummyNode>
Jacobi::MyOperation::dummy_nodes() {
	DummyNode res;
	neighbor_list_t onb;
	res.node = make_node(constant_id, 1 - damp, onb);
	res.type = DummyNodeType::TO_ALL;
	res.func = [=](const id_t& id) { return make_pair(id != constant_id, id); };
	return { res };
}
bool Jacobi::MyOperation::is_dummy_node(const id_t& id) {
	return id == constant_id;
}
Jacobi::value_t Jacobi::MyOperation::func(
    const node_t& n, const neighbor_t& neighbor) {
	// send: -a_ij/a_ii*v_i from j to i
	return -neighbor.second * n.v;
}
// scheduling - priority
priority_t Jacobi::MyOperation::priority(const node_t& n) {
    double p = abs(n.u);
    return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <constant> <use-degree-priority>
AppArguments Jacobi::MySeparator::separate(
    const std::vector<std::string>& args) {
    AppArguments res;
    res.name = Jacobi::name;
    res.operation_arg = {args[0], args[1] };
    res.iohandler_arg = {};
    res.progressor_arg = {};
    return res;
}
