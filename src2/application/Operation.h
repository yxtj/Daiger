#pragma once
#include "common/Node.h"
#include <utility>
#include <string>
#include <vector>

struct OperationBase {
	virtual ~OperationBase() = default;
	// parse the given parameters
	virtual void init(const std::vector<std::string>& args){}
};

template <typename V, typename N>
struct Operation 
	: public OperationBase
{
	typedef V value_t;
	typedef N neighbor_t;
	typedef std::vector<neighbor_t> neighbor_list_t;
	typedef Node<V, N> node_t;

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
	virtual priority_t priority(const node_t& n){ return 0; };

	virtual ~Operation()=default;
};

template <class V, typename W>
struct WeightedOperation :
	public Operation<V, std::pair<id_t, W>>
{};

template <class V>
struct UnWeightedOperation : 
	public Operation<V, id_t>
{};

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
