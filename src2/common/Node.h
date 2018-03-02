#pragma once
#include "def.h"
#include "type_traits/type_traits_dummy.h"
#include <vector>
#include <unordered_map>

template <typename V=float, typename N=std::pair<key_t, float> >
struct Node{
	using value_t = V;
	using neighbor_t = N;
	using neighbor_list_t = std::vector<N>;

	key_t id;
	V v; // value
	V u; // uncommitted value
	// priority_t pri; // priority
	neighbor_list_t onb; // out-neighbors
	std::unordered_map<key_t, V> cs; // caches for in-neighbors
	key_t b; // the best source (for selective)
};

template <typename V, typename N>
bool operator==(const Node<V,N>& a, const Node<V, N>& b){
	return a.id == b.id;
}

template <typename V, typename N>
struct NodeHasher{
	std::size_t operator(const Node<v, N>& n){
		using std::hash;
		return hash<key_t>()(n.id);
    }
};

// -------- static functions for understanding the neighbor type --------

key_t get_key(const key_t& n) { return n; }
template <class T>
key_t get_key(const T& n) { 
	static_assert(impl::type_traits::template_false_type<T>::value, "Neighbor type should be key_t or pair<key_t, W>"); 
}
template <class W>
key_t get_key(const std::pair<key_t, W>& n) { return n.first; }
