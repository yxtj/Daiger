#pragma once

#include "def.h"

#include <string>
#include <vector>
#include <utility>

enum class ChangeEdgeType: char{
	ADD='A',
	REMOVE='R',
	INCREASE='I',
	DECREASE='D'
};

struct IterateKernelBase {
};

struct IterateKernelWeighted : public IterateKernelBase {
	virtual void read_data(std::string& line, key_t& k, std::vector<std::pair<key_t, value_t>>& data) = 0;
	// format: "<key>\t<value>:<delta>"
	virtual void read_init(std::string& line, key_t& k, value_t& delta, value_t& value) = 0;
	
	virtual void init_c(const key_t& k, value_t& delta,D& data) = 0;
	virtual const value_t& default_v() const = 0;
	virtual void init_v(const key_t& k,value_t& v,D& data) = 0;

	virtual bool is_accumuative() const;
	virtual bool is_selective() const;

	virtual void oplus(value_t& a, const value_t& b) = 0;
	virtual bool better(const value_t& a, const value_t& b); // only matters when is_selective() is true

	virtual void process_delta_v(const key_t& k, value_t& dalta,value_t& value, D& data){}
	virtual void priority(value_t& pri, const value_t& value, const value_t& delta, const D& data) = 0;

	virtual value_t g_func(const key_t& k,const value_t& delta,const value_t& value, const D& data, const key_t& dst) = 0;
	virtual void g_func(const key_t& k,const value_t& delta,const value_t& value, const D& data, std::vector<std::pair<key_t, value_t> >* output);

	virtual void read_change(std::string& line, key_t& k, ChangeEdgeType& type, std::vector<std::pair<key_t, value_t>>& change){} // only D[0] make sense

	virtual key_t get_key(const typename D::value_type& d) = 0;
	virtual std::vector<key_t> get_keys(const D& d);

	virtual ~IterateKernel()=default;
};

template <class K, class V, class D>
inline bool IterateKernel<K, V, D>::is_selective() const {
	return false;
}

template <class K, class V, class D>
inline bool IterateKernel<K, V, D>::better(const value_t& a, const value_t& b){
	V temp=a;
	this->accumulate(temp, b);
	return temp==a;
}

template <class K, class V, class D>
void IterateKernel<K, V, D>::g_func(const key_t& k,const value_t& delta,const value_t& value, const D& data,
		std::vector<std::pair<K, V> >* output)
{
	//output->clear();
	for(size_t i=0;i<data.size();++i){
		K dst = get_key(data[i]);
		output->push_back(std::make_pair(dst, g_func(k, delta, value, data, dst)));
	}
}

template <class K, class V, class D>
std::vector<K> IterateKernel<K, V, D>::get_keys(const D& d){
	std::vector<K> keys;
	keys.reserve(d.size());
	for(auto v : d){
		keys.push_back(get_key(v));
	}
	return keys;
}
