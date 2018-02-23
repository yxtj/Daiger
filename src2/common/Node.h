#pragma once
#include "def.h"
#include "NodeBasic.h"
#include <unordered_map>

struct Node:
	public NodeBasic
{
	value_t u; // uncommitted value
	std::unordered_map<key_t, value_t> c; // cache
	priority_t pri;

	Node(const key_t& k, const value_t& v, const value_t& u);
};
