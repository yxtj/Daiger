#include "common/Kernel.h"
#include <numeric>
#include <string>
//#include <algorithm>

struct ConnectedComponent
	: public Kernel<key_t, key_t> 
{
	bool pri_degree;
	bool pri_delta;
	virtual bool parse(const std::vector<std::string>& arg_line);

	virtual std::pair<key_t, neighbor_list_t> load_graph(std::string& line);
	virtual change_t load_change(std::string& line);
	virtual std::pair<key_t, value_t> load_value(std::string& line);
	
	// initialize the starting value
	virtual value_t init_value(const key_t& k, const neighbor_list_t& neighbors);

	virtual const value_t identity_element() const{
		return std::numeric_limits<value_t>::min();
	}
	virtual value_t oplus(value_t& a, const value_t& b){
		//return max(a, b);
		return (a<b)?b:a;
	}
	virtual value_t func(const node_t& n, const neighbor_t& neighbor){
		return n.v;
	}

	virtual bool is_selective(){ return true; }
	virtual bool better(const value_t& a, const value_t& b){
		return a > b;
	}
	
	// scheduling - priority
	virtual priority_t priority(const node_t& n){
		return n.v;
	}
};
