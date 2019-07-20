#pragma once
#include "RemoteUpdater.hpp"
#include "common/Node.h"
#include "common/def_func.h"
#include "application/Operation.h"
#include "msg/messages.h"
#include <algorithm>
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
	void init(operation_t* opt, const size_t id,
		const bool aggregate_message, const bool incremental, const bool cache_free);

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
		//return f_update(from, to, v);
		ru->update(from, to, v);
		return true;
	}

	// collect and remove from the table, format: to, from, v
	//std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect(){
	typename MessageDef<V, N>::MsgVUpdate_t collect(){
		return collect(size());
	}
	typename MessageDef<V, N>::MsgVUpdate_t collect(const size_t num){
		//return f_collect(num);
		return ru->collect(num);
	}

private:
	/*
	bool update_general(const id_t& from, const id_t& to, const value_t& v);
	bool update_accumulative_cb(const id_t& from, const id_t& to, const value_t& v); // cache-based
	bool update_accumulative_cf(const id_t& from, const id_t& to, const value_t& v); // cache-free
	bool update_selective_s(const id_t& from, const id_t& to, const value_t& v); // static-graph
	bool update_selective_d(const id_t& from, const id_t& to, const value_t& v); // dynamic-graph

	typename MessageDef<V, N>::MsgVUpdate_t collect_general(const size_t num);
	typename MessageDef<V, N>::MsgVUpdate_t collect_accumulative(const size_t num); // aggregated
	typename MessageDef<V, N>::MsgVUpdate_t collect_selective(const size_t num); // aggregated
	*/
private:
	operation_t* opt;
	// buffer for remote nodes.
	// vector in general case; optimized to only one element for some cases
	std::unordered_map<id_t, std::vector<std::pair<id_t, value_t>>> cont; // to -> [ <from, v> ]*n
	id_t dummy_worker_id; // used for aggregated message

	RemoteUpdater<V, N>* ru;
	std::function<typename MessageDef<V, N>::MsgVUpdate_t(const size_t)> f_collect;
	std::function<bool(const id_t&, const id_t&, const value_t&)> f_update;
};

template <class V, class N>
void RemoteHolder<V, N>::init(operation_t* opt, const size_t id, 
	const bool aggregate_message, const bool incremental, const bool cache_free)
{
	this->opt = opt;
	if(aggregate_message){
		dummy_worker_id = gen_dummy_id(id);
	}
	ru = RemoteUpdaterFactory<V, N>::gen(opt, cache_free, incremental, aggregate_message);
	ru->init(opt, dummy_worker_id, &cont);
	/*
	if(opt->is_accumulative()){
		if(cache_free){
			f_update = std::bind(&RemoteHolder<V, N>::update_accumulative_cf,
				this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}else{
			f_update = std::bind(&RemoteHolder<V, N>::update_accumulative_cb,
				this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}
		if(aggregate_message){
			f_collect = std::bind(&RemoteHolder<V, N>::collect_accumulative, this, std::placeholders::_1);
		}else{
			f_collect = std::bind(&RemoteHolder<V, N>::collect_general, this, std::placeholders::_1);
		}
	}else if(opt->is_selective()){
		if(aggregate_message){
			if(incremental){
				f_update = std::bind(&RemoteHolder<V, N>::update_selective_d,
					this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			}else{
				f_update = std::bind(&RemoteHolder<V, N>::update_selective_s,
					this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			}
			f_collect = std::bind(&RemoteHolder<V, N>::collect_selective, this, std::placeholders::_1);
		}else{
			f_update = std::bind(&RemoteHolder<V, N>::update_general,
				this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			f_collect = std::bind(&RemoteHolder<V, N>::collect_general, this, std::placeholders::_1);
		}
	}else{
		f_update = std::bind(&RemoteHolder<V, N>::update_general,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		f_collect = std::bind(&RemoteHolder<V, N>::collect_general, this, std::placeholders::_1);
	}*/
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

/*
template <class V, class N>
bool RemoteHolder<V, N>::update_general(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	vec.emplace_back(from, v);
	return true;
}
template <class V, class N>
bool RemoteHolder<V, N>::update_accumulative_cb(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	auto it = std::find_if(vec.begin(), vec.end(), [&](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(it == vec.end()){
		vec.emplace_back(from, v);
		return true;
	}else{
		it->second = v;
		return false;
	}
}
template <class V, class N>
bool RemoteHolder<V, N>::update_accumulative_cf(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	if(vec.empty()){
		vec.emplace_back(from, v);
		return true;
	}else{
		auto& p = vec.front();
		p.second = opt->oplus(p.second, v);
		return false;
	}
}
template <class V, class N>
bool RemoteHolder<V, N>::update_selective_s(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	if(vec.empty()){
		vec.emplace_back(from, v);
		return true;
	}else{
		auto& p = vec.front();
		if(opt->better(v, p.second)){
			p.first = from;
			p.second = v;
		}
		return false;
	}
}
template <class V, class N>
bool RemoteHolder<V, N>::update_selective_d(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	if(vec.empty()){
		vec.emplace_back(from, v);
		return true;
	}else{
		auto& p = vec.front();
		// this is DIFFERENT from that from LocalUpdater
		if(from == p.first){
			p.second = v;
		}else if(opt->better(v, p.second)){
			p.first = from;
			p.second = v;
		}
		return false;
	}
}


template <class V, class N>
typename MessageDef<V, N>::MsgVUpdate_t RemoteHolder<V, N>::collect_general(const size_t num){
	typename MessageDef<V, N>::MsgVUpdate_t res;
	auto it=cont.begin();
	// if a source send more than one message to identical target, only keep the last one
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		auto jt = it->second.begin();
		if(it->second.size() == 1){
			res.emplace_back(jt->first, it->first, jt->second);
		}else{
			std::unordered_map<id_t, int> idx; // source-id -> offset in res
			auto jt_end = it->second.end();
			for(; jt != jt_end; ++jt){
				auto kt = idx.find(jt->first);
				if(kt == idx.end()){
					idx[jt->first] = res.size();
					res.emplace_back(jt->first, it->first, jt->second);
				}else{
					std::get<2>(res[kt->second]) = jt->second;
				}
			}
		}
	}
	if(it == cont.end())
		cont.clear();
	else
		cont.erase(cont.begin(), it);
	return res;
}
template <class V, class N>
typename MessageDef<V, N>::MsgVUpdate_t RemoteHolder<V, N>::collect_accumulative(const size_t num){
	typename MessageDef<V, N>::MsgVUpdate_t res;
	auto it=cont.begin();
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		id_t to = it->first;
		value_t v = it->second.front().second;
		// for most cases, there is only one entry for each node
		if(it->second.size() > 1){
			auto jt = it->second.begin();
			auto jt_end = it->second.end();
			for(++jt; jt != jt_end; ++jt){
				v = opt->oplus(v, jt->second);
			}
		}
		res.emplace_back(dummy_worker_id, to, v);
	}
	if(it == cont.end())
		cont.clear();
	else
		cont.erase(cont.begin(), it);
	return res;
}
template <class V, class N>
typename MessageDef<V, N>::MsgVUpdate_t RemoteHolder<V, N>::collect_selective(const size_t num){
	typename MessageDef<V, N>::MsgVUpdate_t res;
	auto it=cont.begin();
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		id_t to = it->first;
		id_t from = it->second.front().first;
		value_t v = it->second.front().second;
		res.emplace_back(from, to, v);
	}
	if(it == cont.end())
		cont.clear();
	else
		cont.erase(cont.begin(), it);
	return res;
}
*/
