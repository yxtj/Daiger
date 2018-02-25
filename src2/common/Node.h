#pragma once
#include "def.h"
#include <vector>
#include <unordered_map>

template <typename V=float, typename N=std::pair<key_t, float> >
struct Node{
	key_t id;
	V v; // value
	V u; // uncommitted value
	priority_t pri; // priority
	std::vector<N> onb; // out-neighbors
	std::unordered_map<key_t, V> c; // cache for in-neighbors
};

template <V, N>
bool operator==(const Node<V,N>& a, const Node<V, N>& b){
	return a.id == b.id;
}

template <V, N>
struct NodeHasher{
	std::size_t operator(const Node& n){
		using std::hash;
		return hash<key_t>()(n.id);
    }
};

