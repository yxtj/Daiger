#pragma once
#include "common/def.h"
#include "common/def_func.h"
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

// -------- a helper class with some predefined useful IO-functions --------

struct IOHelper {
	// load graph
	// format: "k\ta b c "
	static std::pair<id_t, std::vector<id_t> > load_graph_unweighted(const std::string& line);
	// format: "k\tai,aw bi,bw ci,cw "
	template <typename W>
	static std::pair<id_t, std::vector<id_t, W> > load_graph_weighted(const std::string& line);

	// load graph changes
	// format: <type> is one of A, R, I, D
	// for A and R:
	// line: "<type>\t<src>,<dst>"
	// for I and D:
	// line: "<type>\t<src>,<dst>,<weight>"
	static ChangeEdge<id_t> load_change_unweighted(const std::string& line);
	template <typename W>
	static ChangeEdge<std::pair<id_t, W>> load_change_weighted(const std::string& line);

	// load starting values
	// format: "<key>\t<value>"
	template <typename V>
	static std::pair<id_t, V> load_value(const std::string& line);

	// dump result values
	// format: "<key>\t<value>"
	template <typename V>
	static std::string dump_value(const id_t& k, const V& v);
};

template <typename W>
std::pair<id_t, std::vector<id_t, W> > IOHelper::load_graph_weighted(const std::string& line){
	//line: "k\tai,aw bi,bw ci,cw "
	size_t pos = line.find('\t');
	id_t k = stoid(line.substr(0, pos));
	++pos;

	std::vector<id_t, W> data;
	size_t spacepos;
	while((spacepos = line.find(' ', pos)) != line.npos){
		size_t cut = line.find(',', pos + 1);
		id_t to=stoid(line.substr(pos, cut - pos));
		W weight=boost::lexical_cast<W>(line.substr(cut + 1, spacepos - cut - 1));
		data.emplace_back(to, weight);
		pos = spacepos + 1;
	}
	return std::make_pair(std::move(k), std::move(data));
}

template <typename W>
ChangeEdge<std::pair<id_t, W>> IOHelper::load_change_weighted(const std::string& line){
	// <type> is one of A, R, I, D
	// for A and R:
	// line: "<type>\t<src>,<dst>"
	// for I and D:
	// line: "<type>\t<src>,<dst>,<weight>"
	ChangeEdge<std::pair<id_t, W>> res;
	switch(line[0]){
		case 'A': res.type=ChangeEdgeType::ADD;	break;
		case 'R': res.type=ChangeEdgeType::REMOVE;	break;
		case 'I': res.type=ChangeEdgeType::INCREASE;	break;
		case 'D': res.type=ChangeEdgeType::DECREASE;	break;
		default: throw std::invalid_argument("Cannot parse change line: "+line);
	}
	size_t p1=line.find(',', 2);
	res.src=stoid(line.substr(2,p1-2));
	size_t p2=line.find(',', p1+1);
	id_t dst=stoid(line.substr(p1+1, p2-p1-1));
	W w;
	if(res.type == ChangeEdgeType::INCREASE || res.type == ChangeEdgeType::DECREASE)
		w=boost::lexical_cast<W>(line.substr(p2+1));
	res.dst=std::make_pair(dst, w);
	return res;
}

template <typename V>
std::pair<id_t, V> IOHelper::load_value(const std::string& line){
	// format: "<key>\t<value>"
	size_t p=line.find('\t');
	id_t k = stoid(line.substr(0, p));
	++p;
	V v=boost::lexical_cast<V>(line.substr(p));
	return std::make_pair(k, v);
}

template <typename V>
std::string IOHelper::dump_value(const id_t& k, const V& v){
	// format: "<key>\t<value>"
	return std::to_string(k) + "\t" + std::to_string(v);
}
