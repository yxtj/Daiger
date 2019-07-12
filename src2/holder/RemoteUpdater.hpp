#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include "msg/messages.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Variation: 1) whether to use Aggregated Message Technology or not
//            2) operator type (Gen, Acc, Sel)
//            3) whether to use cache, i.e. cache-based or cache-free (CB, CF)
//            4) whether working on static graph or dynamic graph (S, D)
// Acc -> (CB, CF); Sel -> (S, D)

template <class V, class N>
struct RemoteUpdater {
	using value_t = V;
	using data_t = std::unordered_map<id_t, std::vector<std::pair<id_t, value_t>>>;
	void init(Operation<V, N>* opt, const id_t dummy_worker_id, data_t* pd){
		this->opt = opt;
		this->pd = pd;
		this->dummy_worker_id = dummy_worker_id;
	}

	// update the value for a remote node
	virtual void update(const id_t& from, const id_t& to, const V& m) = 0;
	// collect data in <pd> and format them into messages
	virtual typename MessageDef<V>::MsgVUpdate_t collect(const size_t num) = 0;
	//
	// virtual 

protected:
	Operation<V, N>* opt;
	data_t* pd;
	id_t dummy_worker_id;
};

template <class V, class N>
struct RemoteUpdaterFactory {
	static RemoteUpdater<V, N>* gen(Operation<V, N>* opt,
		const bool cache_free, const bool incremental, const bool aggregated);
};

// -----------
// Non-Aggregated version
template <class V, class N>
struct RuNonAggregated : public RemoteUpdater<V, N> {
	using RemoteUpdater<V, N>::pd;
	virtual void update(const id_t& from, const id_t& to, const V& m){
		auto& vec=(*pd)[to];
		vec.emplace_back(from, m);
	}
	// collect all
	virtual typename MessageDef<V>::MsgVUpdate_t collect(const size_t num){
		typename MessageDef<V>::MsgVUpdate_t res;
		auto it=pd->begin();
		for(size_t i=0; i<num && it!=pd->end(); ++i, ++it){
			id_t to = it->first;
			auto jt_end = it->second.end();
			for(auto jt = it->second.begin(); jt != jt_end; ++jt){
				res.emplace_back(jt->first, to, jt->second);
			}
		}
		if(it == pd->end())
			pd->clear();
		else
			pd->erase(pd->begin(), it);
		return res;
	}
};

// Aggregated versions for all the followings
// -----------
// General operator version
template <class V, class N>
struct RuGen : public RuNonAggregated<V, N>{};

// -----------
// Accumulative operator version
template <class V, class N>
struct RuAcc : public RuNonAggregated<V, N>{};

// cache-based accumulative
template <class V, class N>
struct RuAccCb : public RuAcc<V, N>{
	using RemoteUpdater<V, N>::pd;
	using RemoteUpdater<V, N>::opt;
	using RemoteUpdater<V, N>::dummy_worker_id;
	// 1, use the later one for whose from the identical source
	// 2, aggregate messages to the same destination
	virtual typename MessageDef<V>::MsgVUpdate_t collect(const size_t num){
		typename MessageDef<V>::MsgVUpdate_t res;
		auto it=pd->begin();
		for(size_t i=0; i<num && it!=pd->end(); ++i, ++it){
			id_t to = it->first;
			std::unordered_set<id_t> used;
			V m = opt->identity_element();
			auto jt_end = it->second.rend();
			for(auto jt = it->second.rbegin(); jt != jt_end; ++jt){
				if(used.count(jt->first) == 0){
					m = opt->oplus(m, jt->second);
					used.insert(jt->first);
				}
			}
			res.emplace_back(dummy_worker_id, to, m);
		}
		if(it == pd->end())
			pd->clear();
		else
			pd->erase(pd->begin(), it);
		return res;
	}
};

// cache-free accumulative
template <class V, class N>
struct RuAccCf : public RuAcc<V, N>{
	using RemoteUpdater<V, N>::pd;
	using RemoteUpdater<V, N>::opt;
	using RemoteUpdater<V, N>::dummy_worker_id;
	// accumulate everying directly
	virtual void update(const id_t& from, const id_t& to, const V& m){
		auto& vec=(*pd)[to];
		if(vec.empty()){
			vec.emplace_back(from, m);
		}else{
			auto& p = vec.front();
			p.second = opt->oplus(p.second, m);
		}
	}
	// directly send the accumulated message
	virtual typename MessageDef<V>::MsgVUpdate_t collect(const size_t num){
		typename MessageDef<V>::MsgVUpdate_t res;
		auto it=pd->begin();
		for(size_t i=0; i<num && it!=pd->end(); ++i, ++it){
			id_t to = it->first;
			V v = it->second.front().second;
			res.emplace_back(dummy_worker_id, to, v);
		}
		if(it == pd->end())
			pd->clear();
		else
			pd->erase(pd->begin(), it);
		return res;
	}
};

// -----------
// Selective operator version
template <class V, class N>
struct RuSel : public RuNonAggregated<V, N>{
	using RemoteUpdater<V, N>::pd;
	using RemoteUpdater<V, N>::opt;
	// only send the kept best one
	virtual typename MessageDef<V>::MsgVUpdate_t collect(const size_t num){
		typename MessageDef<V>::MsgVUpdate_t res;
		auto it=pd->begin();
		for(size_t i=0; i<num && it!=pd->end(); ++i, ++it){
			id_t to = it->first;
			id_t from = it->second.front().first;
			V v = it->second.front().second;
			res.emplace_back(from, to, v);
		}
		if(it == pd->end())
			pd->clear();
		else
			pd->erase(pd->begin(), it);
		return res;
	}
};

// static-graph selective
template <class V, class N>
struct RuSelS : public RuSel<V, N>{
	using RemoteUpdater<V, N>::pd;
	using RemoteUpdater<V, N>::opt;
	// only keep the best one
	virtual void update(const id_t& from, const id_t& to, const V& m){
		auto& vec=(*pd)[to];
		if(vec.empty()){
			vec.emplace_back(from, m);
		}else{
			auto& p = vec.front();
			if(opt->better(m, p.second)){
				p.first = from;
				p.second = m;
			}
		}
	}
};

// dynamic-graph selective
template <class V, class N>
struct RuSelD : public RuSel<V, N>{
	using RemoteUpdater<V, N>::pd;
	using RemoteUpdater<V, N>::opt;
	// only keep the best one
	virtual void update(const id_t& from, const id_t& to, const V& m){
		auto& vec=(*pd)[to];
		if(vec.empty()){
			vec.emplace_back(from, m);
		}else{
			auto& p = vec.front();
			// this is DIFFERENT from that from LocalUpdater
			if(from == p.first){
				p.second = m;
			}else if(opt->better(m, p.second)){
				p.first = from;
				p.second = m;
			}
		}
	}
};


// -------- Factory --------

template <class V, class N>
RemoteUpdater<V, N>* RemoteUpdaterFactory<V, N>::gen(Operation<V, N>* opt,
	const bool cache_free, const bool incremental, const bool aggregated)
{
	RemoteUpdater<V, N>* p = nullptr;
	if(!aggregated){
		p = new RuNonAggregated<V,N>();
	}else if(opt->is_accumulative()){
		if(cache_free)
			p = new RuAccCf<V, N>();
		else
			p = new RuAccCb<V, N>();
	}else if(opt->is_selective()){
		if(incremental)
			p = new RuSelD<V, N>();
		else
			p = new RuSelS<V, N>();
	}else{
		p = new RuGen<V, N>();
	}
	return p;
}
