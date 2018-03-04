#pragma once
#include "common/def.h"
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

class IOHandlerBase {
public:
	// load graph changes
	virtual change_t load_change(std::string& line); // default: use IOHelper::load_change
	virtual void init(std::vector<std::string>& args){}
};

template <typename V, typename N>
class IOHandler
	: public IOHandlerBase 
{
public:
	using value_t = V;
	using neighbor_t = N;
	using neighbor_list_t = std::vector<N>;

	// load graph
	virtual std::pair<key_t, neighbor_list_t> load_graph(std::string& line) = 0;
	// load starting values
	virtual std::pair<key_t, value_t> load_value(std::string& line) = 0;
	// dump result
	std::string dump_value(const key_t& k, const valut_t& v);
};

// -------- a helper class with some predefined useful IO-functions --------

struct IOHelper {
	// load graph
	// format: "k\ta b c "
	static std::pair<key_t, std::vector<key_t> > load_graph_unweighted(std::string& line);
	// format: "k\tai,aw bi,bw ci,cw "
	template <typename W>
	static std::pair<key_t, std::vector<key_t, W> > load_graph_weighted(std::string& line);

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
	static std::pair<key_t, V> load_value(std::string& line);

	// dump result values
	// format: "<key>\t<value>"
	template <typename V>
	static std::string dump_value(const key_t& k, const V& v);
};

template <typename W>
std::pair<key_t, std::vector<key_t, W> > load_graph_weighted(std::string& line){
	//line: "k\tai,aw bi,bw ci,cw "
	size_t pos = line.find('\t');
	key_t k = stok(line.substr(0, pos));
	++pos;

	std::vector<key_t, W> data;
	size_t spacepos;
	while((spacepos = line.find(' ', pos)) != line.npos){
		size_t cut = line.find(',', pos + 1);
		key_t to=stok(line.substr(pos, cut - pos));
		W weight=boost::lexical_cast<W>(line.substr(cut + 1, spacepos - cut - 1));
		data.emplace_back(to, weight);
		pos = spacepos + 1;
	}
	return make_pair(move(k), move(data));
}

template <typename V>
std::pair<key_t, V> IOHelper::load_value(std::string& line){
	// format: "<key>\t<value>"
	size_t p=line.find('\t');
	key_t k = stok(line.substr(0, p));
	++p;
	V v=boost::lexical_cast<V>(line.substr(p));
	return make_pair(k, v);
}

template <typename V>
std::string IOHelper::dump_value(const key_t& k, const V& v){
	// format: "<key>\t<value>"
	return stok(k) + "\t" + to_string(v);
}
