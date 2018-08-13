#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include "application/Scheduler.h"
#include "application/Terminator.h"
#include "LocalUpdater.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>

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
	using sender_req_t = std::function<void(const id_t&, const id_t&)>;

	LocalHolder() = default;
	void init(operation_t* opt, scheduler_t* scd, terminator_t* tmt, size_t n,
		bool incremental, bool async, bool cache_free);
	void setUpdateFunction(bool incremental, bool async, bool cache_free);

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
	void registerRequestCallback(sender_req_t f);

	void init_value(const id_t& k, const value_t& v); // used for loading value

	// enumerate nodes
	void enum_rewind();
	const node_t* enum_next(const bool with_dummy = false);
	void enum_sorted_prepare(const bool with_dummy = false);
	void enum_sorted_rewind();
	const node_t* enum_sorted_next();
	
	// -------- node modification functions --------
	bool modify(const id_t& k, const value_t& v); // change value
	bool modify(const id_t& k, const neighbor_list_t& nl); // change neighbor list
	bool modify(const id_t& k, neighbor_list_t&& nl);
	bool modify_onb_add(const id_t& k, const neighbor_t& n); // add an out-neighbor
	bool modify_onb_rmv(const id_t& k, const neighbor_t& n);
	bool modify_onb_val(const id_t& k, const neighbor_t& n);
	bool modify_onb_via_fun_all(const id_t& k, std::function<std::pair<bool, N>(const id_t& k)> func);
	bool modify_cache_add(const id_t& k, const id_t& src, const value_t& v); // add a cache entry
	bool modify_cache_rmv(const id_t& k, const id_t& src);
	bool modify_cache_val(const id_t& from, const id_t& to, const value_t& m);

	// -------- key functions (assume every key exists) --------
	// process the initialization messages
	void cal_initialization(const id_t& from, const id_t& to, const value_t& m);
	// update cache with received message
	void update_cache(const id_t& from, const id_t& to, const value_t& m);
	// merge all caches, the result is stored in <u>
	void cal_general(const id_t& k);
	// update <u> incrementally with <m> from <from> (assume every key exists)
	void cal_incremental(const id_t& from, const id_t& to, const value_t& m);
	// whether <u> is different from <v>
	bool need_commit(const id_t& k) const;
	// update <v> to <u>, update progress, PRECONDITION: the priority of corresponding node is reset before calling commit()
	bool commit(const id_t& k);
	// generate outgoing messages
	std::vector<std::pair<id_t, value_t>> spread(const id_t& k);

	std::vector<std::pair<id_t, value_t>> spread_general(const id_t& k);
	std::vector<std::pair<id_t, value_t>> spread_acf(const id_t& k);
	
	// -------- others --------
	ProgressReport get_progress() const {
		return ProgressReport{progress_value, progress_inf, progress_changed};
	}
	void reset_progress_count(){
		progress_changed = 0;
	}
	size_t get_n_uncommitted() const { return n_uncommitted; }
	bool has_uncommitted() const { return n_uncommitted != 0; }
	void reset_n_uncommitted(){ n_uncommitted = 0; }

private:
	void update_priority(const node_t& n); // when n.u changes. ALSO update n_uncommitted
	void update_progress(const double old_p, const double new_p); // when n.v changes

private:
	//using f_update_general_t = void (LocalHolder::*)(const id_t&);
	//using f_update_incremental_t = void (LocalHolder::*)(const id_t&, const id_t&, const value_t&);
	LocalUpdater<V, N>* plu;

	operation_t* opt;
	scheduler_t* scd;
	terminator_t* tmt;
	bool cache_free;
	std::unordered_map<id_t, node_t> cont;
	//f_update_general_t f_update_general;
	//std::function<void(const id_t&, const id_t&, const value_t&)> f_update_incremental;
	std::function<void(const id_t&, node_t&, const value_t&)> f_update_incremental;
	//f_update_incremental_t f_update_incremental;
	//sender_req_t f_send_req;
	std::function<std::vector<std::pair<id_t, value_t>>(const id_t&)> f_spread;
	std::function<double(const node_t&)> f_priority;

	double progress_value; // summation of the non-infinity value
	size_t progress_inf; // # of the infinity
	size_t progress_changed; // # of changed nodes

	size_t n_uncommitted; // # of uncommitted changes on n.u

	using iterator_t = typename decltype(cont)::const_iterator;
	iterator_t enum_it;
	std::vector<iterator_t> sorted_it_cont;
	typename std::vector<iterator_t>::const_iterator enum_sorted_it;
};

template <class V, class N>
void LocalHolder<V, N>::init(operation_t* opt, scheduler_t* scd, terminator_t* tmt, size_t n,
	bool incremental, bool async, bool cache_free)
{
	this->opt = opt;
	this->scd = scd;
	this->tmt = tmt;
	this->cache_free = cache_free;
	progress_value = 0.0;
	progress_inf = 0;
	progress_changed = 0;

	setUpdateFunction(incremental, async, cache_free);

	if(n != 0)
		cont.reserve(n);
}

template <class V, class N>
void LocalHolder<V, N>::setUpdateFunction(bool incremental, bool async, bool cache_free){
	plu = LocalUpdaterFactory<V, N>::gen(opt, cache_free);
	plu->init(opt);
	if(incremental){
		f_update_incremental = [&](const id_t& from, node_t& n, const value_t& m){
			plu->d_incremental_update(from, n, m);
		};
	}else{
		f_update_incremental = [&](const id_t& from, node_t& n, const value_t& m){
			plu->s_incremental_update(from, n, m);
		};
	}
	if(cache_free && opt->is_accumulative()){
		f_spread = [&](const id_t& k){
			return this->spread_acf(k);
		};
		f_priority = [&](const node_t& n){
			value_t t = n.v;
			node_t& nn = const_cast<node_t&>(n);
			nn.v = opt->identity_element();
			double p = opt->priority(n);
			nn.v = t;
			return p;
		};
	}else{
		f_spread = [&](const id_t& k){
			return this->spread_general(k);
		};
		f_priority = [&](const node_t& n){
			return opt->priority(n);
		};
	}
}

// -------- basic functions --------

template <class V, class N>
void LocalHolder<V, N>::add(const node_t& n){
	update_progress(0.0, tmt->progress(n));
	update_priority(n);
	cont[n.id]=n;
}
template <class V, class N>
void LocalHolder<V, N>::add(node_t&& n){
	update_progress(0.0, tmt->progress(n));
	update_priority(n);
	cont[n.id]=std::move(n);
}
template <class V, class N>
bool LocalHolder<V, N>::remove(const id_t& k){
	auto it = cont.find(k);
	if(it != cont.end()){
		update_progress(tmt->progress(*it), 0.0);
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
void LocalHolder<V, N>::registerRequestCallback(sender_req_t f){
	//f_send_req = f;
	plu->registerRequestCallback(f);
}

template <class V, class N>
void LocalHolder<V, N>::init_value(const id_t& k, const value_t& v){
	node_t& n = get(k);
	double oldp = tmt->progress(n);
	n.v = v;
	update_progress(oldp, tmt->progress(n));
	if(cache_free && opt->is_accumulative()){
		// if(n.u != opt->identity_element()){
		// 	n.u = opt->identity_element();
		// 	update_priority(n);
		// }
	}else{
		n.u = v;
		update_priority(n);
	}
}

template <class V, class N>
void LocalHolder<V, N>::enum_rewind(){
	enum_it = cont.cbegin();
}
template <class V, class N>
const typename LocalHolder<V, N>::node_t* LocalHolder<V, N>::enum_next(const bool with_dummy){
	const node_t* p = nullptr;
	while(!with_dummy && enum_it != cont.cend() && opt->is_dummy_node(enum_it->first)){
		++enum_it;
	}
	if(enum_it != cont.cend()){
		p = &(enum_it->second);
		++enum_it;
	}
	return p;
}

template <class V, class N>
void LocalHolder<V, N>::enum_sorted_prepare(const bool with_dummy){
	sorted_it_cont.clear();
	sorted_it_cont.reserve(cont.size());
	for(auto it = cont.cbegin(); it != cont.cend(); ++it){
		if(with_dummy || !opt->is_dummy_node(it->second.id))
			sorted_it_cont.push_back(it);
	}
	std::sort(sorted_it_cont.begin(), sorted_it_cont.end(), [](iterator_t a, iterator_t b){
		return a->second.id < b->second.id;
	});
}
template <class V, class N>
void LocalHolder<V, N>::enum_sorted_rewind(){
	enum_sorted_it = sorted_it_cont.cbegin();
}
template <class V, class N>
const typename LocalHolder<V, N>::node_t* LocalHolder<V, N>::enum_sorted_next(){
	const node_t* p = nullptr;
	if(enum_sorted_it != sorted_it_cont.cend()){
		p = &((*enum_sorted_it)->second);
		++enum_sorted_it;
	}
	return p;
}

// -------- node modification functions --------

#define MODIFY_TEMPLATE(operation) { auto it=cont.find(k); \
 if(it==cont.end()) return false; operation; return true; }

template <class V, class N>
bool LocalHolder<V, N>::modify(const id_t& k, const value_t& v){
	MODIFY_TEMPLATE(
		double oldp = tmt->progress(*it);
		it->second.v=v;
		update_progress(oldp, tmt->progress(*it));
	)
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
bool LocalHolder<V, N>::modify_onb_via_fun_all(const id_t& k, std::function<std::pair<bool, N>(const id_t& k)> func){
	auto it=cont.find(k);
	if(it==cont.end())
		return false;
	it->second.onb.clear();
	for(auto& p : cont){
		auto t = func(p.first);
		if(t.first)
			it->second.onb.emplace_back(std::move(t.second));
	}
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

template <class V, class N>
void LocalHolder<V, N>::cal_initialization(const id_t& from, const id_t& to, const value_t& m){
	node_t& n=cont[to];
	plu->s_incremental_update(from, n, m);
	update_priority(n);
}
// update cache with received message
template <class V, class N>
void LocalHolder<V, N>::update_cache(const id_t& from, const id_t& to, const value_t& m){
	cont[to].cs[from]=m;
}
template <class V, class N>
void LocalHolder<V, N>::cal_general(const id_t& k){
	node_t& n=cont[k];
	plu->batch_update(n);
	update_priority(n);
}
// update <u> incrementally with <m> from <from> (assume every key exists)
template <class V, class N>
void LocalHolder<V, N>::cal_incremental(const id_t& from, const id_t& to, const value_t& m){
	node_t& n=cont[to];
	f_update_incremental(from, n, m);
	update_priority(n);
}
// whether <u> is different from <v>
template <class V, class N>
bool LocalHolder<V, N>::need_commit(const id_t& k) const{
	const node_t& n=cont.at(k);
	return plu->need_commit(n);
}
// update <v> to <u>
// GUARANTE: the priority of n is reset before calling commit()
template <class V, class N>
bool LocalHolder<V, N>::commit(const id_t& k){
	node_t& n=cont[k];
	if(!plu->need_commit(n))
		return false;
	double oldp = tmt->progress(n);
	plu->commit(n);
	update_progress(oldp, tmt->progress(n));
	return true;
}
// generate outgoing messages
template <class V, class N>
std::vector<std::pair<id_t, V>> LocalHolder<V, N>::spread(const id_t& k){
	return f_spread(k);
}
// general version
template <class V, class N>
std::vector<std::pair<id_t, V>> LocalHolder<V, N>::spread_general(const id_t& k){
	node_t& n=cont[k];
	return opt->func(n);
}
// accumulative operators under cache-free mode
template <class V, class N>
std::vector<std::pair<id_t, V>> LocalHolder<V, N>::spread_acf(const id_t& k){
	node_t& n=cont[k];
	V t = n.v;
	n.v = n.u;
	auto res = opt->func(n);
	n.v = t;
	return res;
}

// -------- others --------

template <class V, class N>
void LocalHolder<V, N>::update_priority(const node_t& n){
	if(plu->need_commit(n)){
		scd->update(n.id, f_priority(n));
		++n_uncommitted;
	}
}

template <class V, class N>
void LocalHolder<V, N>::update_progress(const double old_p, const double new_p){
	if(old_p == new_p)
		return;
	if(std::isinf(old_p)){
		--progress_inf;
	}else{
		progress_value -= old_p;
	}
	if(std::isinf(new_p)){
		++progress_inf;
	}else{
		progress_value += new_p;
	}
	++progress_changed;
}
