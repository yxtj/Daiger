#pragma once
#include <ostream>
#include <utility>
#include <tuple>
#include <vector>
#include <initializer_list>
//#include <array>
#include <map>
#include <list>
#include <deque>
#include <set>
#include <unordered_map>
#include <unordered_set>
//#include "../my_type_traits/is_associated_container.h"

template <class T1, class T2>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
	return os << "(" << p.first << "," << p.second << ")";
}

template <class T1, class T2, class T3>
inline std::ostream& operator<<(std::ostream& os, const std::tuple<T1, T2, T3>& t) {
	return os << "(" << std::get<0>(t) << "," << std::get<1>(t) << "," << std::get<2>(t) << ")";
}

template <class T1, class T2, class T3, class T4>
inline std::ostream& operator<<(std::ostream& os, const std::tuple<T1, T2, T3, T4>& t) {
	return os << "(" << std::get<0>(t) << "," << std::get<1>(t)
		<< "," << std::get<2>(t) << "," << std::get<3>(t) << ")";
}


template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& cont) {
	os << "[";
	for(auto& t : cont)
		os << t << " ";
	return os << "]";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::initializer_list<T>& cont) {
	os << "[";
	for(auto& t : cont)
		os << t << " ";
	return os << "]";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::list<T>& cont) {
	os << "[";
	for(auto& t : cont)
		os << t << " ";
	return os << "]";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::deque<T>& cont) {
	os << "[";
	for(auto& t : cont)
		os << t << " ";
	return os << "]";
}

//template <class T>
//inline std::ostream& operator<<(std::ostream& os, const std::array<T>& cont) {
//	os << "[";
//	for(auto& t : cont)
//		os << t << " ";
//	return os << "]";
//}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::set<T>& cont) {
	os << "{";
	for(auto& t : cont)
		os << t << " ";
	return os << "}";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::multiset<T>& cont) {
	os << "{";
	for(auto& t : cont)
		os << t << " ";
	return os << "}";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& cont) {
	os << "{";
	for(auto& t : cont)
		os << t << " ";
	return os << "}";
}

template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::unordered_multiset<T>& cont) {
	os << "{";
	for(auto& t : cont)
		os << t << " ";
	return os << "}";
}

template <class K, class V>
inline std::ostream& operator<<(std::ostream& os, const std::map<K, V>& cont) {
	os << "{ ";
	for(auto&t : cont)
		os << "(" << t.first << ":" << t.second << ") ";
	return os << "}";
}

template <class K, class V>
inline std::ostream& operator<<(std::ostream& os, const std::multimap<K, V>& cont) {
	os << "{ ";
	for(auto&t : cont)
		os << "(" << t.first << ":" << t.second << ") ";
	return os << "}";
}

template <class K, class V>
inline std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& cont) {
	os << "{ ";
	for(auto&t : cont)
		os << "(" << t.first << ":" << t.second << ") ";
	return os << "}";
}

template <class K, class V>
inline std::ostream& operator<<(std::ostream& os, const std::unordered_multimap<K, V>& cont) {
	os << "{ ";
	for(auto&t : cont)
		os << "(" << t.first << ":" << t.second << ") ";
	return os << "}";
}
