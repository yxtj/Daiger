#pragma once
#include "common/Node.h"
#include "api/api.h"
#include <vector>
#include <unordered_map>
#include <utility>

template <class V, class N>
class RemoteHolder{
    public:
    using kernel_t = Kernel<V, N>;
    using node_t = Node<V, N>;
    using value_t = node_t::value_t;
	using neighbor_t = node_t::neighbor_t;
	using neighbor_list_t = node_t::neighbor_list_t;
    RemoteHolder(kernel_t& kernel);

    bool empty() const;
    size_t size() const;
    void clear();

    bool exist(const key_t& k) const;
    bool exist(const key_t& from, const key_t& to) const;
    bool remove(const key_t& k);
    bool remove(const key_t& from, const key_t& to);
    std::pair<bool, std::vector<std::pair<key_t, value_t>>> get(const key_t& k) const;
    std::pair<bool, value_t> get(const key_t& from, const key_t& to) const;

    // replace the old value with v, return whether a new entry is inserted
	bool set(const key_t& from, const key_t& to, const value_t& v);
    // merge the old value and v using oplus, return whether a new entry is inserted
    bool merge(const key_t& from, const key_t& to, const value_t& v);

    std::vector<std::pair<key_t, std::pair<key_t, valuee_t>>> collect(); // collect and remove from the table
    std::vector<std::pair<key_t, std::pair<key_t, valuee_t>>> collect(const size_t num);

    private:
    Kernel<V, N> knl;
    std::unordered_map<key_t, std::vector<std::pair<key_t, value_t>>> cont;

};

template <class V, class N>
RemoteHolder<V, N>::RemoteHolder(kernel_t& kernel)
    : knl(kernel)
{
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
bool RemoteHolder<V, N>::exist(const key_t& k) const{
    return cont.find(k) != cont.end();
}
template <class V, class N>
bool RemoteHolder<V, N>::exist(const key_t& from, const key_t& to) const{
    auto it=cont.find(to);
    if(it==cont.end())
        return false;
    auto jt=std::find_if(it->second.begin(), it->second.end(), [](const std::pair<key_t, value_t>& p){
        return p.first == from;
    });
    return jt!=it->second.end();
}
template <class V, class N>
bool RemoteHolder<V, N>::remove(const key_t& k){
    return cont.erase(k) != 0;
}
template <class V, class N>
bool RemoteHolder<V, N>::remove(const key_t& from, const key_t& to){
    auto it=cont.find(to);
    if(it==cont.end())
        return false;
    auto jt=std::find_if(it->second.begin(), it->second.end(), [](const std::pair<key_t, value_t>& p){
        return p.first == from;
    });
    if(jt==it->second.end())
        return false;
    it->second.erase(jt);
    return true;
}
template <class V, class N>
std::pair<bool, std::vector<std::pair<key_t, value_t>>> RemoteHolder<V, N>::get(const key_t& k) const{
    auto it = cont.find(k);
    if(it == cont.end()){
        return make_pair(false, {});
    }else{
        return make_pair(true, it->second);
    }
}
template <class V, class N>
std::pair<bool, value_t> RemoteHolder<V, N>::get(const key_t& from, const key_t& to) const{
    auto it=cont.find(to);
    if(it==cont.end())
        return make_pair(false, value_t());
    auto jt=std::find_if(it->second.begin(), it->second.end(), [](const std::pair<key_t, value_t>& p){
        return p.first == from;
    });
    if(jt==it->second.end())
        return make_pair(false, value_t());
    return make_pair(true, jt->second);
}


template <class V, class N>
bool RemoteHolder<V, N>::set(const key_t& from, const key_t& to, const value_t& v){
    auto& vec=cont[to];
    auto jt=std::find_if(vec.begin(), vec.end(), [](const std::pair<key_t, value_t>& p){
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
bool RemoteHolder<V, N>::merge(const key_t& from, const key_t& to, const value_t& v){
    auto& vec=cont[to];
    auto jt=std::find_if(vec.begin(), vec.end(), [](const std::pair<key_t, value_t>& p){
        return p.first == from;
    });
    if(jt==vec.end()){
        vec.emplace_back(from, v);
        return true;
    }else{
        jt->second=knl.oplus(jt->second, v);
        return false;
    }
}
template <class V, class N>
std::vector<std::pair<key_t, std::pair<key_t, valuee_t>>> RemoteHolder<V, N>::collect(){
    return collect(size());
}
template <class V, class N>
std::vector<std::pair<key_t, std::pair<key_t, valuee_t>>> RemoteHolder<V, N>::collect(const size_t num){
    std::vector<std::pair<key_t, std::pair<key_t, valuee_t>>> res;
    auto it=cont.begin();
    for(size_t i=0; i<num && it!=cont.end(); ++i, ++it){
        res.emplace(move(it->first), move(it->second));
    }
    cont.erase(cont.begin(), it);
}

