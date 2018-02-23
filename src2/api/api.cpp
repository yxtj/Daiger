/*
	Provide some basic/default implementation for the APIs
*/
#include "api.h"

using namespace std;

void Kernel::func(const key_t& k, const value_t& v, const neighbor_list_t& neighbors,
	std::vector<std::pair<key_t, value_t> >* output)
{
	if(output == nullptr)
		return;
	for(const neighbor_t& n: neighbors){
		output->emplace_back(n.first, func(k, v, n, n.first));
	}
}

bool Kernel::is_accumuative(){
	return false;
}
bool Kernel::is_selective(){
	return false;
}
// subtype for accumulative
void Kernel::ominus(value_t& a, const value_t& b){
	a-=b;
}
// subtype for selective
bool Kernel::better(const value_t& a, const value_t& b){
	return false;
}

