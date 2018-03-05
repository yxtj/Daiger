#pragma once
#include "common/def.h"
#include "common/def_func.h"
#include <string>
#include <vector>
#include <utility>
#include <boost/lexical_cast.hpp>

// -------- a helper class with some predefined useful IO-functions --------

struct IOHelper {
	// load graph
	// format: "k\ta b c "
	static std::pair<id_t, std::vector<id_t> > load_graph_unweighted(std::string& line);
	// format: "k\tai,aw bi,bw ci,cw "
	template <typename W>
	static std::pair<id_t, std::vector<id_t, W> > load_graph_weighted(std::string& line);

	// load graph changes
	// format: <type> is one of A, R, I, D
	// for A and R:
	// line: "<type>\t<src>,<dst>"
	// for I and D:
	// line: "<type>\t<src>,<dst>,<weight>"
	static change_t load_change(std::string& line);

	// load starting values
	// format: "<key>\t<value>"
	template <typename V>
	static std::pair<id_t, V> load_value(std::string& line);

	// dump result values
	// format: "<key>\t<value>"
	template <typename V>
	static std::string dump_value(const id_t& k, const V& v);
};

template <typename W>
std::pair<id_t, std::vector<id_t, W> > load_graph_weighted(std::string& line){
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

template <typename V>
std::pair<id_t, V> IOHelper::load_value(std::string& line){
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
