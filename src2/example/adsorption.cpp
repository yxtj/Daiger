#include "adsorption.h"
#include "util/Util.h"
#include <algorithm>
#include <vector>

using namespace std;

const std::string Adsorption::name("ad");

std::string Adsorption::getName() const { return name; }

void Adsorption::reg() { AppKernelFactory::registerClass<Adsorption>(name); }

ArgumentSeparator* Adsorption::generateSeparator() { return new separator_t(); }
OperationBase* Adsorption::generateOperation() { return new operation_t(); }
IOHandlerBase* Adsorption::generateIOHandler() { return new iohandler_t(); }
TerminatorBase* Adsorption::generateTerminator() { return new terminator_t(); }
GlobalHolderBase* Adsorption::generateGraph() { return new graph_t(); }

// -------- Components --------

void Adsorption::MyOperation::init(const std::vector<std::string>& arg_line) {
    damp = stod(arg_line[0]);
    if (damp < 0.0 || damp > 1.0) {
        throw invalid_argument("Invalid damping factor: " + arg_line[0]);
    }
    use_degree = beTrueOption(arg_line[1]);
    dummy_id = -1;
}
Adsorption::MyOperation::node_t Adsorption::MyOperation::preprocess_node(
    const id_t& k, neighbor_list_t& neighbors) {
    return make_node(k, 0.0, neighbors);
}
std::vector<Adsorption::MyOperation::DummyNode>
Adsorption::MyOperation::dummy_nodes() {
    DummyNode res;
    neighbor_list_t onb;
    res.node = make_node(dummy_id, 1 - damp, onb);
    res.type = DummyNodeType::TO_ALL;
    //res.func = [=](const id_t& id) { return make_pair(id != dummy_id, id); };
    return {res};
}
bool Adsorption::MyOperation::is_dummy_node(const id_t& id) {
    return id == dummy_id;
}
Adsorption::value_t Adsorption::MyOperation::func(
    const node_t& n, const neighbor_t& neighbor) {
    return n.id != dummy_id ? damp * n.v / n.onb.size() : 1 - damp;
}
// scheduling - priority
priority_t Adsorption::MyOperation::priority(const node_t& n) {
    double p = abs(n.u - n.v);
    return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <damp-factor> <use-degree-priority> <epsilon-termination>
AppArguments Adsorption::MySeparator::separate(
    const std::vector<std::string>& args) {
    AppArguments res;
    res.name = Adsorption::name;
    res.operation_arg = {args[0], args[1]};
    res.iohandler_arg = {};
    res.terminator_arg = {args[2]};
    return res;
}