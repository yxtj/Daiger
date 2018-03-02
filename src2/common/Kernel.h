#pragma once
#include "common/Node.h"
#include "type_traits/type_traits_dummy.h"
#include <utility>
#include <string>
#include <vector>

struct KernelBase {};

template <typename V, typename N>
struct Kernel 
	: public KernelBase
{
	using value_t = V;
	using neighbor_t = N;
	using neighbor_list_t = std::vector<N>;
	using node_t = Node<V, N>;

	// understand the neighbor type
	key_t get_key(const key_t& n) { return n; }
	template <class T>
	key_t get_key(const T& n) { 
		static_assert(impl::type_traits::template_false_type<T>::value, "Neighbor type should be key_t or pair<key_t, W>"); 
	}
	template <class W>
	key_t get_key(const std::pair<key_t, W>& n) { return n.first; }

	// parse the given parameters
	virtual bool parse(const std::vector<std::string>& arg_line){}

	// load graph
	virtual std::pair<key_t, neighbor_list_t> load_graph(std::string& line) = 0;
	// load graph changes
	virtual change_t load_change(std::string& line) = 0;
	// load starting values
	virtual std::pair<key_t, value_t> load_value(std::string& line) = 0;
	
	// initialize the starting value
	virtual value_t init_value(const key_t& k, const neighbor_list_t& neighbors) = 0;

	// operations: identity_element, oplus, f-function
	virtual value_t identity_element() const = 0; // identity_element
	virtual value_t oplus(const value_t& a, const value_t& b) = 0; // merge function: eg. a+b or min(a,b)
	virtual value_t func(const node_t& n, const neighbor_t& neighbor) = 0; // transition function for a message, only on given <neighbor> instead local variable

	// helper operations: group-level f-function, ominus for accumulative, better for selective
	virtual std::vector<std::pair<key_t, value_t>> func(const node_t& n); // for all neighbors, default: use the previous func for all
	virtual bool is_accumuative(); // default: false
	virtual bool is_selective(); // default: false
	// subtype for accumulative
	virtual void ominus(value_t& a, const value_t& b); // when is_accumuative() is true, default: a-=b
	// subtype for selective
	virtual bool better(const value_t& a, const value_t& b); // when is_selective() is true, default: false
	
	// scheduling - priority
	virtual priority_t priority(const node_t& n) = 0;
	virtual bool prioritized(const priority_t a, const priority_t b); // default: a > b

	virtual ~Kernel()=default;
};

template <class V, typename W>
struct WeightedKernel :
	public Kernel<V, std::pair<key_t, W>>
{};

template <class V>
struct UnWeightedKernel : 
	public Kernel<V, key_t>
{};

template <class V, class N>
std::vector<std::pair<key_t, value_t>> Kernel::func(const node_t& n)
{
	std::vector<std::pair<key_t, value_t>> output;
	output.reserve(n.neighbors.size());
	for(const auto& dst: neighbors){
		output.emplace_back(get_key(dst), func(n, dst));
	}
}

template <class V, class N>
bool Kernel<V, N>::is_accumuative(){
	return false;
}
template <class V, class N>
bool Kernel<V, N>::is_selective(){
	return false;
}
template <class V, class N>
void Kernel<V, N>::ominus(value_t& a, const value_t& b){
	a-=b;
}
template <class V, class N>
bool Kernel<V, N>::better(const value_t& a, const value_t& b){
	return false;
}

template <class V, class N>
bool Kernel<V, N>::prioritized(const priority_t a, const priority_t b){
	return a>b;
}
