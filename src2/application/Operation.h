#pragma once
#include "common/Node.h"
#include <utility>
#include <string>
#include <vector>
#include <limits>

struct OperationBase {
	virtual ~OperationBase() = default;
	// parse the given parameters
	virtual void init(const std::vector<std::string>& args){}
};

template <typename V, typename N>
struct Operation 
	: public OperationBase
{
	using value_t = V;
	using neighbor_t = N;
	using node_t = Node<V, N>;
	using neighbor_list_t = typename node_t::neighbor_list_t;

	// initialize the starting value
	virtual value_t init_value(const id_t& k, const neighbor_list_t& neighbors) = 0;

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
	
	// priority for scheduling
	virtual priority_t priority(const node_t& n){ return n.u; }; // default: current uncommitted value

	virtual ~Operation()=default;
};

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

	virtual value_t identity_element() const{ return 0; }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a - b; }
	virtual value_t ominus(const value_t& a, const value_t& b){ return a + b; }
};

template <typename V, typename N>
struct OperationMin
	: public OperationSelective<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return std::numeric_limits<value_t>::max(); }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a < b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a < b; }
};
template <typename V, typename N>
struct OperationMinEqual
	: public OperationSelective<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return std::numeric_limits<value_t>::max(); }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a <= b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a <= b; }
};
template <typename V, typename N>
struct OperationMax
	: public OperationSelective<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return std::numeric_limits<value_t>::lowest(); }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a > b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a > b; }
};
template <typename V, typename N>
struct OperationMaxEqual
	: public OperationSelective<V, N>
{
	using value_t = V;

	virtual value_t identity_element() const{ return std::numeric_limits<value_t>::lowest(); }
	virtual value_t oplus(const value_t& a, const value_t& b){ return a >= b ? a : b; }
	virtual bool better(const value_t& a, const value_t& b){ return a >= b; }
};
