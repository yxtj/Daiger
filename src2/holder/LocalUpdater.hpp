#pragma once
#include "common/Node.h"
#include "application/Operation.h"

template <class V, class N>
struct LocalUpdater {
	void init(Operation<V, N>* opt){ this->opt = opt; }
	// for cache-free selective; parameter: request-from, request-to
	virtual void registerRequestCallback(std::function<void(const id_t&, const id_t&)> freq){}
	// update on a static graph for batch input
	virtual V batch_update(Node<V, N>& n) = 0;
	// update on a static graph for incremental input
	virtual V s_incremental_update(const id_t& from, Node<V, N>& n, const V& m) = 0;
	// update on a dynamic graph for incremental input
	virtual V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m) = 0;
	virtual bool need_commit(const Node<V, N>& n){
		return n.v != n.u;
	}
	virtual void commit(Node<V, N>& n){
		n.v = n.u;
	}
protected:
	Operation<V, N>* opt;
};

template <class V, class N>
struct LocalUpdaterFactory {
	static LocalUpdater<V, N>* gen(Operation<V, N>* opt, const bool cache_free);
};

// -----------
// Cache-based
template <class V, class N>
struct LuCacheBased : public LocalUpdater<V, N>{};

// General operator, Cache-based
template <class V, class N>
struct LuCbGen : public LuCacheBased<V, N>{
	virtual V batch_update(Node<V, N>& n){
		V tmp = this->opt->identity_element();
		for(auto& p : n.cs){
			tmp = this->opt->oplus(tmp, p.second);
		}
		n.u = tmp;
		return tmp;
	}
	virtual V s_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.cs[from] = m;
		n.u = this->opt->oplus(n.u, m);
		return n.u;
	}
	virtual V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.cs[from] = m;
		return batch_update(n);
	}
};
// Accumulative operator, Cache-based
template <class V, class N>
struct LuCbAcc : public LuCbGen<V, N>{
	V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.u = this->opt->oplus(this->opt->ominus(n.u, n.cs[from]), m);
		n.cs[from] = m;
		return n.u;
	}
};
// Selective operator, Cache-based
template <class V, class N>
struct LuCbSel : public LuCbGen<V, N>{
	virtual V batch_update(Node<V, N>& n){
		V tmp = this->opt->identity_element();
		id_t bp;
		for(auto& c : n.cs){
			if(this->opt->better(c.second, tmp)){
				tmp=c.second;
				bp=c.first;
			}
		}
		n.b = bp;
		n.u = tmp;
		return tmp;
	}
	virtual V s_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		if(this->opt->better(m, n.u)){
			n.u = m;
			n.b = from;
		}
		n.cs[from] = m;
		return n.u;
	}
	virtual V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.cs[from] = m;
		if(this->opt->better(m, n.u)){
			n.u = m;
			n.b = from;
		}else if(from == n.b){
			batch_update(n);
		}
		return n.u;
	}
};

// -----------
// Cache-free
template <class V, class N>
struct LuCacheFree : public LuCacheBased<V, N>{};

// General operator, Cache-free
template <class V, class N>
struct LuCfGen : public LuCbGen<V, N>{};
// Accumulative operator, Cache-free
template <class V, class N>
struct LuCfAcc : public LuCbAcc<V, N>{
	V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.u = this->opt->oplus(n.u, m);
		return n.u;
	}
	virtual bool need_commit(const Node<V, N>& n){
		return n.u != 0;
	}
	virtual void commit(Node<V, N>& n){
		n.v = this->opt->oplus(n.v, n.u);
		n.u = this->opt->identity_element();
	}
};
// Selective operator, Cache-free
template <class V, class N>
struct LuCfSel : public LuCbSel<V, N>{
	virtual void registerRequestCallback(std::function<void(const id_t&, const id_t&)> f){
		f_req = f;
	}
	virtual V s_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		if(this->opt->better(m, n.u)){
			n.u = m;
			n.b = from;
		}
		return n.u;
	}
	virtual V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		if(this->opt->better(m, n.u)){
			n.u = m;
			n.b = from;
		}else if(from == n.b){
			n.u = m;
			for(const auto& p : n.cs){
				f_req(n.id, p.first);
			}
		}
		return n.u;
	}
protected:
	std::function<void(const id_t&, const id_t&)> f_req; // request-from, request-to
};

// --------
// factory
template <class V, class N>
LocalUpdater<V, N>* LocalUpdaterFactory<V, N>::gen(Operation<V, N>* opt, const bool cache_free){
	LocalUpdater<V, N>* p = nullptr;
	if(opt->is_accumulative()){
		if(cache_free)
			p = new LuCfAcc<V, N>();
		else
			p = new LuCbAcc<V, N>();
	}else if(opt->is_selective()){
		if(cache_free)
			p = new LuCfSel<V, N>();
		else
			p = new LuCbSel<V, N>();
	}else{
		if(cache_free)
			p = new LuCbGen<V, N>();
		else
			p = new LuCfGen<V, N>();
	}
	return p;
}

