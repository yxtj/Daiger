#pragma once
#include "IOHandler_Helper.h"
#include "common/def.h"
#include <string>
#include <vector>
#include <utility>

class IOHandlerBase {
public:
	virtual void init(const std::vector<std::string>& args){}
	// load graph changes
	virtual change_t load_change(std::string& line){
		return IOHelper::load_change(line);
	}
};

struct IOHelper;

template <typename V, typename N>
class IOHandler
	: public IOHandlerBase 
{
public:
	typedef V value_t;
	typedef N neighbor_t;
	typedef std::vector<N> neighbor_list_t;

	// load graph
	virtual std::pair<id_t, neighbor_list_t> load_graph(std::string& line) = 0;
	// load starting values
	virtual std::pair<id_t, value_t> load_value(std::string& line) {
		return IOHelper::load_value<value_t>(line);
	}
	// dump result
	virtual std::string dump_value(const id_t& k, const value_t& v) {
		return IOHelper::dump_value<value_t>(k, v);
	}
};

template <typename V>
class IOHandlerUnweighted
	: public IOHandler<V, id_t> 
{
public:
	using typename IOHandler<V, id_t>::neighbor_list_t;
	virtual std::pair<id_t, neighbor_list_t> load_graph(std::string& line){
		return IOHelper::load_graph_unweighted(line);
	}
};

template <typename V, typename N>
class IOHandlerWeighted
	: public IOHandler<V, N>
{
public:
	using typename IOHandler<V, N>::neighbor_list_t;
	virtual std::pair<id_t, neighbor_list_t> load_graph(std::string& line){
		return IOHelper::load_graph_weighted<typename N::second_type>(line);
	}
};
