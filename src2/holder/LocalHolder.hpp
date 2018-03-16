#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include "application/Scheduler.h"
#include "application/Terminator.h"
#include <vector>
#include <unordered_map>
#include <functional>

class LocalHolderBase{};

template <class V, class N>
class LocalHolder
	: public LocalHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using scheduler_t = SchedulerBase;
	using terminator_t = Terminator<V, N>;
	using node_t = Node<V, N>;
	using value_t = V; //typename node_t::value_t;
	using neighbor_t = typename node_t::neighbor_t;
	using neighbor_list_t = typename node_t::neighbor_list_t;

	LocalHolder() = default;
	void init(operation_t* opt, scheduler_t* scd, terminator_t* tmt, size_t n);

	// -------- basic functions --------
	void add(const node_t& n);
	void add(node_t&& n);
	bool remove(const id_t& k);
	bool exist(const id_t& k);
	node_t& get(const id_t& k);
	const node_t& get(const id_t& k) const;
	bool empty() const;
	size_t size() const;
	void clear();
	// enumerate nodes
	void enum_rewind();
	const node_t* enum_next();
	
	// -------- node modification functions --------
	bool modify(const id_t& k, const value_t& v); // change value
	bool modify(const id_t& k, const neighbor_list_t& nl); // change neighbor list
	bool modify(const id_t& k, neighbor_list_t&& nl);
	bool modify_onb_add(const id_t& k, const neighbor_t& n); // add an out-neighbor
	bool modify_onb_rmv(const id_t& k, const neighbor_t& n);
	bool modify_onb_val(const id_t& k, const neighbor_t& n);
	bool modify_cache_add(const id_t& k, const id_t& src, const value_t& v); // add a cache entry
	bool modify_cache_rmv(const id_t& k, const id_t& src);
	bool modify_cache_val(const id_t& from, const id_t& to, const value_t& m);

	// -------- key functions (assume every key exists) --------
	void update_cache(const id_t& from, const id_t& to, const value_t& m); // update cache with received message
	void cal_general(const id_t& k); // merge all caches, the result is stored in <u>
	bool need_commit(const id_t& k) const; // whether <u> is different from <v>
	bool commit(const id_t& k); // update <v> to <u>, update progress, REQUIRE: the priority of corresponding node is reset before calling commit()
	std::vector<std::pair<id_t, value_t>> spread(const id_t& k); // generate outgoing messages

	// -------- incremental update functions (assume every key exists, assume the cache is not updated by m) --------
	void cal_incremental(const id_t& from, const id_t& to, const value_t& m)
		{ f_update_incremental(from, to, m); }
	void inc_cal_general(const id_t& from, const id_t& to, const value_t& m); // incremental update using recalculate
	void inc_cal_accumulative(const id_t& from, const id_t& to, const value_t& m); // incremental update
	void inc_cal_selective(const id_t& from, const id_t& to, const value_t& m); // incremental update
	
	// -------- others --------
	void update_priority(const node_t& n);
	double get_progress() { return progress; }
	
private:
	operation_t* opt;
	scheduler_t* scd;
	terminator_t* tmt;
	double progress;
	std::unordered_map<id_t, node_t> cont;
	std::function<void(const id_t&, const id_t&, const value_t&)> f_update_incremental;

	typename decltype(cont)::const_iterator enum_it;
};

template <class V, class N>
void LocalHolder<V, N>::init(operation_t* opt, scheduler_t* scd, terminator_t* tmt, size_t n)
{
	this->opt = opt;
	this->scd = scd;
	this->tmt = tmt;
	progress = 0.0;
	if(opt->is_accumulative()){
		f_update_incremental = std::bind(&LocalHolder<V, N>::inc_cal_accumulative,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}else if(opt->is_selective()){
		f_update_incremental = std::bind(&LocalHolder<V, N>::inc_cal_selective,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}else{
		f_update_incremental = std::bind(&LocalHolder<V, N>::inc_cal_general,
			this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}
	if(n != 0)
		cont.reserve(n);
}


// -------- basic functions --------

template <class V, class N>
void LocalHolder<V, N>::add(const node_t& n){
	progress += tmt->progress(n);
	cont[n.id]=n;
}
template <class V, class N>
void LocalHolder<V, N>::add(node_t&& n){
	progress += tmt->progress(n);
	cont[n.id]=std::move(n);
}
template <class V, class N>
bool LocalHolder<V, N>::remove(const id_t& k){
	auto it = cont.find(k);
	if(it != cont.end()){
		progress -= tmt->progress(it->second);
		return true;
	}
	return false;
}
template <class V, class N>
bool LocalHolder<V, N>::exist(const id_t& k){
	return cont.find(k) != cont.end();
}
template <class V, class N>
typename LocalHolder<V, N>::node_t& LocalHolder<V, N>::get(const id_t& k){
	return cont.at(k);
}
template <class V, class N>
const typename LocalHolder<V, N>::node_t& LocalHolder<V, N>::get(const id_t& k) const{
	return cont.at(k);
}
template <class V, class N>
bool LocalHolder<V, N>::empty() const{
	return cont.empty();
}
template <class V, class N>
size_t LocalHolder<V, N>::size() const{
	return cont.size();
}
template <class V, class N>
void LocalHolder<V, N>::clear(){
	cont.clear();
}
template <class V, class N>
void LocalHolder<V, N>::enum_rewind(){
	enum_it = cont.cbegin();
}
template <class V, class N>
const typename LocalHolder<V, N>::node_t* LocalHolder<V, N>::enum_next(){
	const node_t* p = nullptr;
	if(enum_it != cont.end()){
		p = &(enum_it->second);
		++enum_it;
	}
	return p;
}

// -------- node modification functions --------

#define MODIFY_TEMPLATE(operation) { auto it=cont.find(k); \
 if(it==cont.end()) return false; operation; return true; }

template <class V, class N>
bool LocalHolder<V, N>::modify(const id_t& k, const value_t& v){
	MODIFY_TEMPLATE( it->second.v=v; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify(const id_t& k, const neighbor_list_t& nl){
	MODIFY_TEMPLATE( it->second.onb=nl; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify(const id_t& k, neighbor_list_t&& nl){
	MODIFY_TEMPLATE( it->second.onb=std::move(nl); )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_onb_add(const id_t& k, const neighbor_t& n){
	MODIFY_TEMPLATE( it->second.onb.push_back(n); )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_onb_rmv(const id_t& k, const neighbor_t& n){
	auto it=cont.find(k);
	if(it==cont.end())
		return false;
	auto jt = std::find_if(it->second.onb.begin(), it->second.onb.end(), [&](const N& nb){
		return get_key(nb) == get_key(n);
	});
	if(jt==it->second.onb.end())
		return false;
	it->second.onb.erase(jt);
	return true;
}
template <class V, class N>
bool LocalHolder<V, N>::modify_onb_val(const id_t& k, const neighbor_t& n){
	auto it=cont.find(k);
	if(it==cont.end())
		return false;
	auto jt = std::find_if(it->second.onb.begin(), it->second.onb.end(), [&](const N& nb){
		return get_key(nb) == get_key(n);
	});
	if(jt==it->second.onb.end())
		return false;
	*jt = n;
	return true;
}
template <class V, class N>
bool LocalHolder<V, N>::modify_cache_add(const id_t& k, const id_t& src, const value_t& v){
	MODIFY_TEMPLATE( it->second.cs[src]=v; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_cache_rmv(const id_t& k, const id_t& src){
	MODIFY_TEMPLATE( return it->second.cs.erase(src) != 0; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_cache_val(const id_t& from, const id_t& to, const value_t& m){
	auto it=cont.find(to);
	if(it==cont.end())
		return false;
	auto jt = it->second.cs.find(from);
	if(jt==it->second.cs.end())
		return false;
	jt->second = m;
	return true;
}

// -------- key functions (assume every key exists) --------

// update cache with received message
template <class V, class N>
void LocalHolder<V, N>::update_cache(const id_t& from, const id_t& to, const value_t& m){
	cont[to].cs[from]=m;
}
// merge all caches, the result is stored in <u>
template <class V, class N>
void LocalHolder<V, N>::cal_general(const id_t& k){
	node_t& n=cont[k];
	value_t tmp=opt->identity_element();
	for(auto& p : n.cs){
		tmp = opt->oplus(tmp, p.second);
	}
	n.u = tmp;
	update_priority(n);
}
// whether <u> is different from <v>
template <class V, class N>
bool LocalHolder<V, N>::need_commit(const id_t& k) const{
	const node_t& n=cont[k];
	return n.v == n.u;
}
// update <v> to <u>
// GUARANTE: the priority of n is reset before calling commit()
template <class V, class N>
bool LocalHolder<V, N>::commit(const id_t& k){
	node_t& n=cont[k];
	if(n.v == n.u)
		return false;
	double oldp = tmt->progress(n);
	n.v = n.u;
	progress += tmt->progress(n) - oldp;
	return true;
}
// generate outgoing messages
template <class V, class N>
std::vector<std::pair<id_t, V>> LocalHolder<V, N>::spread(const id_t& k){
	node_t& n=cont[k];
	return opt->func(n);
}


// -------- incremental update functions --------

// incremental update using recalculation
template <class V, class N>
void LocalHolder<V, N>::inc_cal_general(const id_t& from, const id_t& to, const value_t& m){
	node_t& n=cont[to];
	value_t tmp=opt->identity_element();
	for(auto& c : n.cs){
		if(c.first != from)
			tmp = opt->oplus(tmp, c.second);
		else
			tmp = opt->oplus(tmp, m);
	}
	n.u = tmp;
	update_priority(n);
}
// incremental update for cache-based accumulative
template <class V, class N>
void LocalHolder<V, N>::inc_cal_accumulative(const id_t& from, const id_t& to, const value_t& m){
	node_t& n=cont[to];
	n.u = opt->oplus( opt->ominus(n.u, n.cs[from]), m);
	update_priority(n);
}
// incremental update for cache-based selective
template <class V, class N>
void LocalHolder<V, N>::inc_cal_selective(const id_t& from, const id_t& to, const value_t& m){
	node_t& n=cont[to];
	value_t old=n.u;
	if(opt->better(m, old)){
		n.u = m;
		n.b = from;
		update_priority(n);
	}else if(from == n.b){
		value_t tmp=opt->identity_element();
		id_t bp;
		for(auto& c : n.cs){
			const value_t& v = c.first!=from?c.second:m;
			if(opt->better(v, tmp)){
				tmp=v;
				bp=c.first;
			}
		}
		n.u = tmp;
		n.b = bp;
		update_priority(n);
	}
}
 
// -------- others --------

template <class V, class N>
void LocalHolder<V, N>::update_priority(const node_t& n){
	scd->update(n.id, opt->priority(n));
}

