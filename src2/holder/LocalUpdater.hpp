#pragma once
#include "common/Node.h"
#include "application/Operation.h"

template <class V, class N>
struct LocalUpdater {
	LocalUpdaterBase(operation_t* opt):opt(opt){}
	virtual void registerRequestCallback(std::function<void(const int, std::string&)> freq){}
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
	operation_t* opt;
}

// General operator, Cache-based
template <class V, class N>
struct LuCacheBased : public LocalUpdater<V, N>{}

template <class V, class N>
V LuCacheBased<V, N>::batch_update(Node<V, N>& n){
	value_t tmp = opt->identity_element();
	for(auto& p : n.cs){
		tmp = opt->oplus(tmp, p.second);
	}
	n.u = tmp;
	return tmp;
}
template <class V, class N>
V LuCacheBased<V, N>::s_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
	n.cs[from] = m;
	n.u = opt->oplus(n.u, m);
	return n.u;
}
template <class V, class N>
V LuCacheBased<V, N>::d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
	n.cs[from] = m;
	return batch_update(n);
}

// Accumulative operator, Cache-based
template <class V, class N>
struct LuCbAcc : public LuCacheBased<V, N>{
	V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.u = opt->oplus(opt->ominus(n.u, n.cs[from]), m)
		n.cs[from] = m;
		return n.u;
	}
}
// Selective operator, Cache-based
template <class V, class N>
struct LuCbSel : public LuCacheBased<V, N>{
	virtual V batch_update(Node<V, N>& n){
		value_t tmp=opt->identity_element();
		id_t bp;
		for(auto& c : n.cs){
			if(opt->better(c.second, tmp)){
				tmp=c.second;
				bp=c.first;
			}
		}
		n.b = bp;
		n.u = tmp;
		return tmp;
	}
	virtual V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.cs[from] = m;
		if(opt->better(m, n.u)){
			n.u = m;
			n.b = from;
		}else if(from == n.b){
			batch_update(n);
		}
		return n.u;
	}
}

// -----------

// General operator, Cache-based
template <class V, class N>
struct LuCacheFree : public LocalUpdater<V, N>{}

// Accumulative operator, Cache-free
template <class V, class N>
struct LuCfAcc : public LuCacheFree<V, N>{
	V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.u = opt->oplus(n.u, m);
	}
	virtual bool need_commit(const Node<V, N>& n){
		return n.u != 0;
	}
	virtual void commit(Node<V, N>& n){
		n.v = opt->oplus(n.v, n.u);
		n.u = opt->identity_element();
	}
}
// Selective operator, Cache-free
template <class V, class N>
struct LuCfSel : public LuCacheFree<V, N>{
	virtual void registerRequestCallback(std::function<void(const int, std::string&)> f){
		f_req = f;
	}
	V d_incremental_update(const id_t& from, Node<V, N>& n, const V& m){
		n.u = opt->oplus(n.u, m);
	}
protected:
	std::function<void(const int, std::string&)> f_req;
}

