#pragma once
#include "def.h"
#include "def_func.h"
#include "type_traits/type_traits_dummy.h"
#include <vector>
#include <unordered_map>

template <typename V=float, typename N=std::pair<id_t, float> >
struct Node{
	typedef V value_t;
	typedef N neighbor_t;
	typedef std::vector<N> neighbor_list_t;

	id_t id;
	V v; // value
	V u; // uncommitted value
	// priority_t pri; // priority
	//priority_t pf; // factor for priority
	neighbor_list_t onb; // out-neighbors
	neighbor_list_t inb; // in-neighbors
	std::unordered_map<id_t, V> cs; // caches for in-neighbors
	id_t b; // the best source (for selective)
};

template <typename V, typename N>
bool operator==(const Node<V,N>& a, const Node<V, N>& b){
	return a.id == b.id;
}

template <typename V, typename N>
struct NodeHasher{
	std::size_t operator()(const Node<V, N>& n){
		using std::hash;
		return hash<id_t>()(n.id);
    }
};

enum class DummyNodeType {
	NORMAL,
	TO_ALL
};

// -------- static functions for understanding the neighbor type --------

inline id_t get_key(const id_t& n) {
	return n;
}
template <class T>
inline id_t get_key(const T& n) { 
	static_assert(impl::type_traits::template_false_type<T>::value, "Neighbor type should be id_t or pair<id_t, W>");
	return 0;
}
template <class W>
inline id_t get_key(const std::pair<id_t, W>& n) {
	return n.first;
}

inline id_t make_in_neighbor(const id_t& from, const id_t& to)
{
	return from;
}
template <class T>
inline T make_in_neighbor(const id_t& from, const T& to) {
	static_assert(impl::type_traits::template_false_type<T>::value, "Neighbor type should be id_t or pair<id_t, W>");
	return T();
}
template <class W>
inline std::pair<id_t, W> make_in_neighbor(const id_t& from, const std::pair<id_t, W>& to) {
	return std::make_pair(from, to.second);
}

