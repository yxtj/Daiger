#include "IOHandler.h"
#include <boost/lexical_cast.hpp>
#include <exception>

using namespace std;

change_t IOHandlerBase::load_change(std::string& line){
	return IOHelper::load_change(line);
}

// -------- IOHelper --------

// load graph
std::pair<key_t, std::vector<key_t> > IOHelper::load_graph_unweighted(std::string& line){
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

// load graph changes
change_t IOHelper::load_change(std::string& line){
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
