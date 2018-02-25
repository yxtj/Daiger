#pragma once
#include "common/Node.h"
#include <utility>
#include <string>
#include <vector>

template <V, N>
struct Kernel {
	using value_t = V;
	using neighbor_t = N;
	using neighbor_list_t = std::vector<N>;

	// understand the neighbor type
	virtual key_t get_key(const N& n) = 0;

	// load graph
	virtual std::pair<key_t, neighbor_list_t> load_graph(std::string& line) = 0;
	// load graph changes
	virtual change_t load_change(std::string& line) = 0;
	// load starting values
	virtual std::pair<key_t, value_t> load_value(std::string& line) = 0;
	
	// initialize the starting value
	virtual value_t init_value(const key_t& k, const neighbor_list_t& neighbors) = 0;

	// operations: identity_element, oplus, f-function
	virtual const value_t& default_v() const = 0; // identity_element
	virtual void oplus(value_t& a, const value_t& b) = 0; // a += b or a=min(a,b)
	virtual value_t func(const key_t& k, const value_t& v, const neighbor_t& neighbor,
		const key_t& dst) = 0; // generate one out-going message

	// helper operations: group-level f-function, ominus for accumulative, better for selective
	virtual void func(const key_t& k, const value_t& v, const neighbor_list_t& neighbors,
		std::vector<std::pair<key_t, value_t> >* output); // use the prervious one by default
	virtual bool is_accumuative(); // default -> false
	virtual bool is_selective(); // default -> false
	// subtype for accumulative
	virtual void ominus(value_t& a, const value_t& b); // only matters when is_accumuative() is true
	// subtype for selective
	virtual bool better(const value_t& a, const value_t& b); // only matters when is_selective() is true
	
	// scheduling - priority
	virtual priority_t priority(const key_t& t, const value_t& value, const neighbor_list_t& neighbors) = 0;
	virtual bool prioritized(const priority_t a, const priority_t b); // default a > b

	virtual ~Kernel()=default;
}

template <V, N>
void Kernel::func(const key_t& k, const value_t& v, const neighbor_list_t& neighbors,
	std::vector<std::pair<key_t, value_t> >* output)
{
	if(output == nullptr)
		return;
	for(const auto& n: neighbors){
		output->emplace_back(get_key(n), func(k, v, n, get_key(n)));
	}
}

template <V, N>
bool Kernel::is_accumuative(){
	return false;
}
template <V, N>
bool Kernel::is_selective(){
	return false;
}
template <V, N>
void Kernel::ominus(value_t& a, const value_t& b){
	a-=b;
}
template <V, N>
bool Kernel::better(const value_t& a, const value_t& b){
	return false;
}

template <V, N>
bool Kernel::prioritized(const priority_t a, const priority_t b){
	return a>b;
}
