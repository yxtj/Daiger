#pragma once
#include "common/def.h"
#include <string>
#include <vector>

struct Kernel {
	// load graph
	virtual std::pair<key_t, neighbor_list_t> read_data(std::string& line) = 0;
	// load graph changes
	virtual change_t read_change(std::string& line) = 0;
	
	// initialize values
	virtual void init_value(const key_t& k, const neighbor_list_t& neighbors) = 0;

	// operations
	virtual const value_t& default_v() const = 0; // identity_element
	virtual void oplus(value_t& a, const value_t& b) = 0; // a += b or a=min(a,b)
	virtual value_t func(const key_t& k, const value_t& v, const neighbor_t& neighbor,
		const key_t& dst) = 0; // generate one out-going message
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

	virtual ~Kernel()=default;
}
