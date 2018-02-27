#include "io.h"
#include <boost/lexical_cast.hpp>
#include <exception>

using namespace std;

namespace common{

// load graph
std::pair<key_t, std::vector<key_t> > load_graph_unweighted(std::string& line){
	//line: "k\ta b c "
	size_t pos = line.find('\t');
	key_t k = stok(line.substr(0, pos));
	++pos;

	vector<key_t> data;
	size_t spacepos;
	while((spacepos = line.find(' ',pos)) != line.npos){
		key_t to = stok(line.substr(pos, spacepos-pos));
		data.push_back(to);
		pos=spacepos+1;
	}
	return make_pair(move(k), move(data));
}

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
		key_t node=stok(line.substr(pos, cut - pos));
		W weight=boost::lexical_cast<W>(line.substr(cut + 1, spacepos - cut - 1));
		data.emplace_back(node, weight);
		pos = spacepos + 1;
	}
	return make_pair(move(k), move(data));
}

// load graph changes
change_t load_change(std::string& line){
	// <type> is one of A, R, I, D
	// for A and R:
	// line: "<type>\t<src>,<dst>"
	// for I and D:
	// line: "<type>\t<src>,<dst>,<weight>"
	change_t res;
	switch(line[0]){
		case 'A': res.type=ChangeEdgeType::ADD;	break;
		case 'R': res.type=ChangeEdgeType::REMOVE;	break;
		case 'I': res.type=ChangeEdgeType::INCREASE;	break;
		case 'D': res.type=ChangeEdgeType::DECREASE;	break;
		default: throw invalid_argument("Cannot parse change line: "+line);
	}
	size_t p1=line.find(',', 2);
	res.src=stok(line.substr(2,p1-2));
	size_t p2=line.find(',', p1+1);
	res.dst=stok(line.substr(p1+1, p2-p1-1));
	if(res.type == ChangeEdgeType::INCREASE || res.type == ChangeEdgeType::DECREASE)
		res.weight=stof(line.substr(p2+1));
	return res;
}

// load starting values
template <typename V>
std::pair<key_t, V> load_value(std::string& line){
	// format: "<key>\t<value>"
	size_t p=line.find('\t');
	key_t k = stok(line.substr(0, p));
	++p;
	V v=boost::lexical_cast<V>(line.substr(p));
	return make_pair(k, v);
}

// dump result values
template <typename V>
std::string dump_value(const key_t& k, const V& v){
	// format: "<key>\t<value>"
	return stok(k) + "\t" + to_string(v);
}

} // namespace
