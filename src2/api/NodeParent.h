#include "def.h"

struct NodeParent {
	key_t key;
	value_t v;
	value_t priority;
	
	
	virtual value_t fun(const key_t& dst);
	virtual value_t oplus(const value_t& v);

};
