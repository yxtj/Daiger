#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>

class RemoteHolderBase {};

template <class V, class N>
class RemoteHolder
	: public RemoteHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using node_t = Node<V, N>;
	using value_t = V; //typename node_t::value_t;
	using neighbor_t = typename node_t::neighbor_t;
	using neighbor_list_t = typename node_t::neighbor_list_t;

	RemoteHolder() = default;
	void init(operation_t* opt);

	bool empty() const;
	size_t size() const;
	void clear();

	bool exist(const id_t& k) const;
	bool exist(const id_t& from, const id_t& to) const;
	bool remove(const id_t& k);
	bool remove(const id_t& from, const id_t& to);
	std::pair<bool, std::vector<std::pair<id_t, value_t>>> get(const id_t& k) const;
	std::pair<bool, value_t> get(const id_t& from, const id_t& to) const;

	// corresponding to update_cache of LocalTable, return whether a new entry is inserted
	bool update(const id_t& from, const id_t& to, const value_t& v){
		return f_update(from, to, v);
	}

	// collect and remove from the table, format: to, from, v
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect(){
		return collect(size());
	}
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect(const size_t num){
		return f_collect(num);
	}

private:
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect_general(const size_t num);
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect_accumulative(const size_t num);
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect_selective(const size_t num);

	bool update_general(const id_t& from, const id_t& to, const value_t& v);
	bool update_accumulative(const id_t& from, const id_t& to, const value_t& v);
	bool update_selective(const id_t& from, const id_t& to, const value_t& v);

private:
	operation_t* opt;
	// TODO: consider merge the <from>s
	std::unordered_map<id_t, std::vector<std::pair<id_t, value_t>>> cont; // to -> [ <from, v> ]*n

	std::function<std::vector<std::pair<id_t, std::pair<id_t, value_t>>>(const size_t)> f_collect;
	std::function<bool(const id_t&, const id_t&, const value_t&)> f_update;
};

template <class V, class N>
void RemoteHolder<V, N>::init(operation_t* opt)
{
	this->opt = opt;
	if(opt->is_accumulative()){
		f_collect = std::bind(&RemoteHolder<V, N>::collect_accumulative, this, std::placeholders::_1);
		f_update = std::bind(&RemoteHolder<V, N>::update_accumulative,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}else if(opt->is_selective()){
		f_collect = std::bind(&RemoteHolder<V, N>::collect_selective, this, std::placeholders::_1);
		f_update = std::bind(&RemoteHolder<V, N>::update_selective,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}else{
		f_collect = std::bind(&RemoteHolder<V, N>::collect_general, this, std::placeholders::_1);
		f_update = std::bind(&RemoteHolder<V, N>::update_general,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}
}

template <class V, class N>
bool RemoteHolder<V, N>::empty() const{
	return cont.empty();
}
template <class V, class N>
size_t RemoteHolder<V, N>::size() const{
	return cont.size();
}
template <class V, class N>
void RemoteHolder<V, N>::clear(){
	cont.clear();
}

template <class V, class N>
bool RemoteHolder<V, N>::exist(const id_t& k) const{
	return cont.find(k) != cont.end();
}
template <class V, class N>
bool RemoteHolder<V, N>::exist(const id_t& from, const id_t& to) const{
	auto it=cont.find(to);
	if(it==cont.end())
		return false;
	auto jt=std::find_if(it->second.begin(), it->second.end(), [&](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	return jt!=it->second.end();
}
template <class V, class N>
bool RemoteHolder<V, N>::remove(const id_t& k){
	return cont.erase(k) != 0;
}
template <class V, class N>
bool RemoteHolder<V, N>::remove(const id_t& from, const id_t& to){
	auto it=cont.find(to);
	if(it==cont.end())
		return false;
	auto jt=std::find_if(it->second.begin(), it->second.end(), [&](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==it->second.end())
		return false;
	it->second.erase(jt);
	return true;
}
template <class V, class N>
std::pair<bool, std::vector<std::pair<id_t, V>>> RemoteHolder<V, N>::get(const id_t& k) const{
	auto it = cont.find(k);
	if(it == cont.end()){
		return std::make_pair<bool, std::vector<std::pair<id_t, V>>>(false, {});
	}else{
		return std::make_pair(true, it->second);
	}
}
template <class V, class N>
std::pair<bool, V> RemoteHolder<V, N>::get(const id_t& from, const id_t& to) const{
	auto it=cont.find(to);
	if(it==cont.end())
		return std::make_pair(false, value_t());
	auto jt=std::find_if(it->second.begin(), it->second.end(), [&](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==it->second.end())
		return std::make_pair(false, value_t());
	return std::make_pair(true, jt->second);
}


template <class V, class N>
bool RemoteHolder<V, N>::update_general(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	vec.emplace_back(from, v);
	return true;
}
template <class V, class N>
bool RemoteHolder<V, N>::update_accumulative(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	auto jt=std::find_if(vec.begin(), vec.end(), [&](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==vec.end()){
		vec.emplace_back(from, v);
		return true;
	}else{
		jt->second = opt->oplus(jt->second, v);
		return false;
	}
}
template <class V, class N>
bool RemoteHolder<V, N>::update_selective(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	auto jt=std::find_if(vec.begin(), vec.end(), [&](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==vec.end()){
		vec.emplace_back(from, v);
		return true;
	}else{
		jt->second = v;
		return false;
	}
}


template <class V, class N>
std::vector<std::pair<id_t, std::pair<id_t, V>>> RemoteHolder<V, N>::collect_general(const size_t num){
	std::vector<std::pair<id_t, std::pair<id_t, V>>> res;
	auto it=cont.begin();
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		auto jt_end = it->second.end();
		for(auto jt = it->second.begin(); jt != jt_end; ++jt){
			res.emplace_back(it->first, std::move(*jt));
		}
	}
	if(it == cont.end())
		cont.clear();
	else
		cont.erase(cont.begin(), it);
	return res;
}
template <class V, class N>
std::vector<std::pair<id_t, std::pair<id_t, V>>> RemoteHolder<V, N>::collect_accumulative(const size_t num){
	std::vector<std::pair<id_t, std::pair<id_t, V>>> res;
	auto it=cont.begin();
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		value_t v = opt->identity_element();
		auto jt_end = it->second.end();
		for(auto jt = it->second.begin(); jt != jt_end; ++jt){
			v = opt->oplus(v, jt->second);
		}
		res.emplace_back(it->first, std::make_pair(it->second.begin()->first, v));
	}
	if(it == cont.end())
		cont.clear();
	else
		cont.erase(cont.begin(), it);
	return res;
}
template <class V, class N>
std::vector<std::pair<id_t, std::pair<id_t, V>>> RemoteHolder<V, N>::collect_selective(const size_t num){
	std::vector<std::pair<id_t, std::pair<id_t, V>>> res;
	auto it=cont.begin();
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		value_t b = opt->identity_element();
		id_t p;
		auto jt_end = it->second.end();
		for(auto jt = it->second.begin(); jt != jt_end; ++jt){
			if(opt->better(jt->second, b)){
				b = jt->second;
				p = jt->first;
			}
		}
		res.emplace_back(it->first, std::make_pair(p, b));
	}
	if(it == cont.end())
		cont.clear();
	else
		cont.erase(cont.begin(), it);
	return res;
}
