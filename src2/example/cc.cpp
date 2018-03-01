#include "cc.h"
#include <string>
#include <numeric>
//#include <algorithm>

using namespace std;

bool ConnectedComponent::Operation::parse(const std::vector<std::string>& arg_line){

	return true;
}

value_t ConnectedComponent::Operation::identity_element() const{
	return std::numeric_limits<value_t>::min();
}
value_t ConnectedComponent::Operation::oplus(value_t& a, const value_t& b){
	//return max(a, b);
	return (a<b)?b:a;
}
value_t ConnectedComponent::Operation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v;
}
bool ConnectedComponent::Operation::better(const value_t& a, const value_t& b){
	return a > b;
}
// scheduling - priority
priority_t ConnectedComponent::Operation::priority(const node_t& n){
	return n.v;
}