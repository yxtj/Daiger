#pragma once
#include "def.h"

struct NodeBasic{
	key_t id;
	value_t v; // value
	neighbor_list_t neighbors;

	NodeBasic(const key_t& k): id(k) {}
	NodeBasic(const key_t& k, const value_t& v): id(k), v(v) {}
};
