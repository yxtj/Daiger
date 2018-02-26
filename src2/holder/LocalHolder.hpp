#pragma once
#include "common/Node.h"
#include "api/api.h"
#include <vector>
#include <unordered_map>
#include <functional>

template <class V, class N>
class LocalHolder{
    public:
    using kernel_t = Kernel<V, N>;
    using node_t = Node<V, N>;
    using value_t = node_t::value_t;
	using neighbor_t = node_t::neighbor_t;
	using neighbor_list_t = node_t::neighbor_list_t;
    LocalHolder(kernel_t& kernel);

    // -------- basic functions --------
	bool add(const node_t& n);
    bool add(node_t&& n);
    bool remove(const key_t& k);
    bool exist(const key_t& k);
    node_t& get(const key_t& k);
    const node_t& get(const key_t& k) const;
    bool empty() const;
    size_t size() const;
    void clear();
    
    // -------- node modification functions --------
    bool modify(const key_t& k, const valut_t& v); // change value
    bool modify(const key_t& k, const neighbor_list_t& nl); // change neighbor list
    bool modify(const key_t& k, neighbor_list_t&& nl);
    bool modify_onb_add(const key_t& k, const neighbor_t& n); // add an out-neighbor
    bool modify_onb_rmv(const key_t& k, const neighbor_t& n);
    bool modify_cache_add(const key_t& k, const key_t& src, const value_t& v); // add a cache entry
    bool modify_cache_rmv(const key_t& k, const key_t& src);

    // -------- key functions (assume every key exists) --------
    void update_cache(const key_t& from, const key_t& to, const value_t& m); // update cache with received message
    void cal_general(const key_t& k); // merge all caches, the result is stored in <u>
    bool need_commit(const key_t& t) const; // whether <u> is different from <v>
    bool commit(const key_t& k); // update <v> to <u>
    std::vector<std::pair<key_t, value_t>> spread(const key_t& k); // generate outgoing messages

    // -------- incremetnal update functions (assume every key exists, assume the cache is not updated by m) --------
    void cal_incremental(onst key_t& from, const key_t& to, const value_t& m)
        { f_update_incremental(from, to, m); }
    void inc_cal_general(const key_t& from, const key_t& to, const value_t& m); // incremental update using recalculate
    void inc_cal_accumulative(const key_t& from, const key_t& to, const value_t& m); // incremental update
    void inc_cal_selective(const key_t& from, const key_t& to, const value_t& m); // incremental update
    
    // -------- others --------
    void update_priority(const key_t& k);
	
    private:
    Kernel<V, N> knl;
    std::unordered_map<key_t, node_t, NodeHasher<V, N>> cont;
    std::function<void(onst key_t&, const key_t&, const value_t&)> f_update_incremental;

};

template <class V, class N>
LocalHolder<V, N>::LocalHolder(kernel_t& kernel)
    : knl(kernel)
{
    if(knl.is_accumuative()){
        f_update_incremental = bind(
            &LocalHolder<V, N>::inc_update_accumulative, this, placeholders::_1, placeholders::_2);
    }else i f(knl.is_selective()){
        f_update_incremental = bind(
            &LocalHolder<V, N>::inc_update_selective, this, placeholders::_1, placeholders::_2);
    }else{
        f_update_incremental = bind(
            &LocalHolder<V, N>::inc_update_general, this, placeholders::_1, placeholders::_2);
    }
}

// -------- basic functions --------

template <class V, class N>
bool LocalHolder<V, N>::add(const node_t& n){
    cont[n.id]=n;
}
template <class V, class N>
bool LocalHolder<V, N>::add(node_t&& n){
    cont[n.id]=move(n);
}
template <class V, class N>
bool LocalHolder<V, N>::remove(const key_t& k){
    return cont.erase(k) != 0;
}
template <class V, class N>
bool LocalHolder<V, N>::exist(const key_t& k){
    return cont.find(k) != cont.end();
}
template <class V, class N>
node_t& LocalHolder<V, N>::get(const key_t& k){
    return cont.at(k);
}
template <class V, class N>
const node_t& LocalHolder<V, N>::get(const key_t& k) const{
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

// -------- node modification functions --------

#define MODIFY_TEMPLATE(operation) { auto it=cont.find(k); \
 if(it==cont.end()) return false; operation; return true; }

template <class V, class N>
bool LocalHolder<V, N>::modify(const key_t& k, const valut_t& v){
    MODIFY_TEMPLATE( it->second.v=v; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify(const key_t& k, const neighbor_list_t& nl){
    MODIFY_TEMPLATE( it->second.onb=nl; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify(const key_t& k, neighbor_list_t&& nl){
    MODIFY_TEMPLATE( it->second.onb=move(nl); )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_onb_add(const key_t& k, const neighbor_t& n){
    MODIFY_TEMPLATE( it->second.onb.append(n); )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_onb_rmv(const key_t& k, const neighbor_t& n){
    MODIFY_TEMPLATE( 
        auto jt = std::find_if(it->second.onb.begin(), it->second.onb.end(), [](const N& nb){
            return knl.get_key(nb) == knl.get_key(n);
        });
        if(jt==it->second.onb.end())
            return false;
        it->second.onb.erase(jt);
    )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_cache_add(const key_t& k, const key_t& src, const value_t& v){
    MODIFY_TEMPLATE( it->second.c[src]=v; )
}
template <class V, class N>
bool LocalHolder<V, N>::modify_cache_rmv(const key_t& k, const key_t& src){
    MODIFY_TEMPLATE( return it->second.c.erase(src) != 0; )
}

// -------- key functions (assume every key exists) --------

// update cache with received message
template <class V, class N>
void LocalHolder<V, N>::update_cache(const key_t& from, const key_t& to, const value_t& m){
    cont[to].cs[from]=m;
}
// merge all caches, the result is stored in <u>
template <class V, class N>
void LocalHolder<V, N>::cal_general(const key_t& k){
    node_t& n=cont[k];
    value_t tmp=knl.identity_element();
    for(auto& p : n.cs){
        tmp = knl.oplus(tmp, p.second);
    }
    n.u = tmp;
}
// whether <u> is different from <v>
template <class V, class N>
bool LocalHolder<V, N>::need_commit(const key_t& t) const{
    const node_t& n=cont[k];
    return n.v == n.u;
}
// update <v> to <u>
template <class V, class N>
bool LocalHolder<V, N>::commit(const key_t& k){
    node_t& n=cont[k];
    if(n.v == n.u)
        return false;
    n.v = n.u;
    return true;
}
// generate outgoing messages
template <class V, class N>
std::vector<std::pair<key_t, value_t>> LocalHolder<V, N>::spread(const key_t& k){
    node_t& n=cont[k];
    return knl.func(k);
}


// -------- incremetnal update functions --------

// incremental update using recalculation
template <class V, class N>
void LocalHolder<V, N>::inc_cal_general(const key_t& from, const key_t& to, const value_t& m){
    node_t& n=cont[to];
    value_t tmp=knl.identity_element();
    for(auto& c : n.cs){
        if(c.first != from)
            tmp = knl.oplus(tmp, c.second);
        else
            tmp = knl.oplus(tmp, m);
    }
    n.u = tmp;
}
// incremental update for cache-based accumulative
template <class V, class N>
void LocalHolder<V, N>::inc_cal_accumulative(const key_t& from, const key_t& to, const value_t& m){
    node_t& n=cont[to];
    n.u = knl.oplus( knl.ominus(n.u, n.cs[from]), m)
}
// incremental update for cache-based selective
template <class V, class N>
void LocalHolder<V, N>::inc_cal_selective(const key_t& from, const key_t& to, const value_t& m){
    node_t& n=cont[to];
    value_t old=n.u;
    if(knl.better(m, old)){
        n.u = m;
        n.b = from;
    }else if(from == n.b){
        value_t tmp=knl.identity_element();
        key_t bp;
        for(auto& c : n.cs){
            const value_t& v = c.first!=from?c.second:m;
            if(knl.better(v, tmp)){
                tmp=v;
                bp=c.first;
            }
        }
        n.u = tmp;
        n.b = bp;
    }
}
 
// -------- others --------

template <class V, class N>
void LocalHolder<V, N>::update_priority(const key_t& k){
    node_t& n=cont[k];
    n.pri = knl.priority(n);
}

