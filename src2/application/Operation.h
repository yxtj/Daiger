#pragma once
#include "common/Node.h"
#include <utility>
#include <tuple>
#include <string>
#include <vector>
#include <limits>
#include <functional>

class OperationBase {
public:
	virtual ~OperationBase() = default;
	// parse the given parameters
	virtual void init(const std::vector<std::string>& args, const size_t nInstance){}
	virtual bool is_accumulative(){ return false; }
	virtual bool is_selective(){ return false; }
};

template <typename V, typename N>
struct Operation 
	: public OperationBase
{
	using value_t = V;
	using neighbor_t = N;
	using node_t = Node<V, N>;
	using neighbor_list_t = typename node_t::neighbor_list_t;

	// <node-body-without-neighbors, type, function-of-adding-neighbors>
	struct DummyNode{
		node_t node; // containing <id>, <u>, <v>
		DummyNodeType type;
		std::function<std::pair<bool, N>(const id_t&)> func; // return wether to add a neighbor to the given id
	};

	// all node-level preprocess including value initialization, out-neighbor adjusting.
	virtual node_t preprocess_node(const id_t& k, neighbor_list_t& neighbors) = 0; // use make_node() to make
	// prepare for output. like normalization
	virtual value_t postprocess_value(const node_t& n){ return n.v; }

	// generate dummy nodes. 
	virtual std::vector<DummyNode> dummy_nodes(); // default: empty
	virtual bool is_dummy_node(const id_t& id) { return false; };

	// operations: identity_element, oplus, f-function
	virtual value_t identity_element() const = 0; // identity_element
	virtual value_t oplus(const value_t& a, const value_t& b) = 0; // merge function: eg. a+b or min(a,b)
	virtual value_t func(const node_t& n, const neighbor_t& neighbor) = 0; // transition function for a message, only on given <neighbor> instead local variable

	// helper operations: group-level f-function, ominus for accumulative, better for selective
	virtual std::vector<std::pair<id_t, value_t>> func(const node_t& n); // for all neighbors, default: use the previous func for all
	virtual bool is_accumulative(); // default: false
	virtual bool is_selective(); // default: false
	// subtype for accumulative
	virtual value_t ominus(const value_t& a, const value_t& b); // when is_accumulative() is true, default: a-b
	// subtype for selective
	virtual bool better(const value_t& a, const value_t& b); // when is_selective() is true, default: false
	
	// priority for scheduling,  default: current uncommitted value
	virtual priority_t priority(const node_t& n){
		return static_cast<priority_t>(n.u);
	}

	virtual ~Operation()=default;

protected:
	// make a node with IE as <v> and given <k>, <u>, <onb>.
	node_t make_node(const id_t& k, value_t&& u, neighbor_list_t& neighbors);
	node_t make_node(const id_t& k, const value_t& u, neighbor_list_t& neighbors);
};

template <class V, class N>
std::vector<typename Operation<V, N>::DummyNode> Operation<V, N>::dummy_nodes(){
	return {};
}
template <class V, class N>
Node<V, N> Operation<V, N>::make_node(const id_t& k, value_t&& u, neighbor_list_t& neighbors){
	node_t n;
	n.id = k;
	n.v = identity_element();
	n.u = std::move(u);
	n.onb = std::move(neighbors);
	return n;
}
template <class V, class N>
Node<V, N> Operation<V, N>::make_node(const id_t& k, const value_t& u, neighbor_list_t& neighbors){
	value_t temp = u;
	return make_node(k, std::move(temp), neighbors);
}

template <class V, class N>
std::vector<std::pair<id_t, V>> Operation<V, N>::func(const node_t& n)
{
	std::vector<std::pair<id_t, value_t>> output;
	output.reserve(n.onb.size());
	for(const auto& dst: n.onb){
		output.emplace_back(get_key(dst), func(n, dst));
	}
	return output;
}

template <class V, class N>
bool Operation<V, N>::is_accumulative(){
	return false;
}
template <class V, class N>
bool Operation<V, N>::is_selective(){
	return false;
}
template <class V, class N>
V Operation<V, N>::ominus(const value_t& a, const value_t& b){
	return a - b;
}
template <class V, class N>
bool Operation<V, N>::better(const value_t& a, const value_t& b){
	return false;
}

// -------- Examples --------
template <typename V, typename N>
struct OperationAccumulative
	: public Operation<V, N>
{
	virtual bool is_accumulative(){ return true; }
};
template <typename V, typename N>
struct OperationSelective
	: public Operation<V, N>
{
	virtual bool is_selective(){ return true; }
};

template <typename V, typename N>
struct OperationAddition
	: public OperationAccumulative<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return 0; }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a + b; }
	virtual value_t ominus(const value_t& a, const value_t& b){ return a - b; }
};
template <typename V, typename N>
struct OperationSubtraction
	: public OperationAccumulative<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return 0; }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a - b; }
	virtual value_t ominus(const value_t& a, const value_t& b){ return a + b; }
};
template <typename V, typename N>
struct OperationMultiplication
	: public OperationAccumulative<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return 1; }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a * b; }
	virtual value_t ominus(const value_t& a, const value_t& b){ return a / b; }
};

// -------- Selective --------

template <typename V, typename N, bool HAS_INF = true>
struct OperationMinBase
	: public OperationSelective<V, N>
{
	virtual V identity_element() const{ return std::numeric_limits<V>::infinity(); }
};
template <typename V, typename N>
struct OperationMinBase<V, N, false>
	: public OperationSelective<V, N>
{
	virtual V identity_element() const{ return std::numeric_limits<V>::max(); }
};

template <typename V, typename N>
struct OperationMin
	: public OperationMinBase<V, N, std::numeric_limits<V>::has_infinity>
{
	using value_t = V;

	virtual value_t oplus(const value_t& a, const value_t& b){ return a < b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a < b; }
};
template <typename V, typename N>
struct OperationMinEqual
	: public OperationMinBase<V, N, std::numeric_limits<V>::has_infinity>
{
	using value_t = V;

	virtual value_t oplus(const value_t& a, const value_t& b){ return a <= b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a <= b; }
};


template <typename V, typename N, bool HAS_INF = true>
struct OperationMaxBase
	: public OperationSelective<V, N>
{
	virtual V identity_element() const{ return -std::numeric_limits<V>::infinity(); }
};
template <typename V, typename N>
struct OperationMaxBase<V, N, false>
	: public OperationSelective<V, N>
{
	virtual V identity_element() const{ return std::numeric_limits<V>::lowest(); }
};

template <typename V, typename N>
struct OperationMax
	: public OperationMaxBase<V, N, std::numeric_limits<V>::has_infinity>
{
	using value_t = V;

	virtual value_t oplus(const value_t& a, const value_t& b){ return a > b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a > b; }
};
template <typename V, typename N>
struct OperationMaxEqual
	: public OperationMaxBase<V, N, std::numeric_limits<V>::has_infinity>
{
	using value_t = V;

	virtual value_t oplus(const value_t& a, const value_t& b){ return a >= b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a >= b; }
};
