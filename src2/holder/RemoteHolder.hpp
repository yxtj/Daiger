#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include <vector>
#include <unordered_map>
#include <utility>

class RemoteHolderBase {};

template <class V, class N>
class RemoteHolder
	: public RemoteHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using node_t = Node<V, N>;
	using value_t = node_t::value_t;
	using neighbor_t = node_t::neighbor_t;
	using neighbor_list_t = node_t::neighbor_list_t;

	RemoteHolder() = default;
	explicit RemoteHolder(operation_t* opt);
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

	// replace the old value with v, return whether a new entry is inserted
	bool set(const id_t& from, const id_t& to, const value_t& v);
	// merge the old value and v using oplus, return whether a new entry is inserted
	bool merge(const id_t& from, const id_t& to, const value_t& v);

	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect(); // collect and remove from the table
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> collect(const size_t num);

private:
	operation_t* opt;
	std::unordered_map<id_t, std::vector<std::pair<id_t, value_t>>> cont;

};

template <class V, class N>
RemoteHolder<V, N>::RemoteHolder(operation_t* opt)
	: opt(opt)
{
}
template <class V, class N>
void RemoteHolder<V, N>::init(operation_t* opt)
{
	this->opt = opt;
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
	auto jt=std::find_if(it->second.begin(), it->second.end(), [](const std::pair<id_t, value_t>& p){
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
	auto jt=std::find_if(it->second.begin(), it->second.end(), [](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==it->second.end())
		return false;
	it->second.erase(jt);
	return true;
}
template <class V, class N>
std::pair<bool, std::vector<std::pair<id_t, value_t>>> RemoteHolder<V, N>::get(const id_t& k) const{
	auto it = cont.find(k);
	if(it == cont.end()){
		return make_pair(false, {});
	}else{
		return make_pair(true, it->second);
	}
}
template <class V, class N>
std::pair<bool, value_t> RemoteHolder<V, N>::get(const id_t& from, const id_t& to) const{
	auto it=cont.find(to);
	if(it==cont.end())
		return make_pair(false, value_t());
	auto jt=std::find_if(it->second.begin(), it->second.end(), [](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==it->second.end())
		return make_pair(false, value_t());
	return make_pair(true, jt->second);
}


template <class V, class N>
bool RemoteHolder<V, N>::set(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	auto jt=std::find_if(vec.begin(), vec.end(), [](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==vec.end()){
		vec.emplace_back(from, v);
		return true;
	}else{
		jt->second=v;
		return false;
	}
}
template <class V, class N>
bool RemoteHolder<V, N>::merge(const id_t& from, const id_t& to, const value_t& v){
	auto& vec=cont[to];
	auto jt=std::find_if(vec.begin(), vec.end(), [](const std::pair<id_t, value_t>& p){
		return p.first == from;
	});
	if(jt==vec.end()){
		vec.emplace_back(from, v);
		return true;
	}else{
		jt->second=opt->oplus(jt->second, v);
		return false;
	}
}
template <class V, class N>
std::vector<std::pair<id_t, std::pair<id_t, valuee_t>>> RemoteHolder<V, N>::collect(){
	return collect(size());
}
template <class V, class N>
std::vector<std::pair<id_t, std::pair<id_t, valuee_t>>> RemoteHolder<V, N>::collect(const size_t num){
	std::vector<std::pair<id_t, std::pair<id_t, valuee_t>>> res;
	auto it=cont.begin();
	for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
		res.emplace(move(it->first), move(it->second));
	}
	cont.erase(cont.begin(), it);
}

