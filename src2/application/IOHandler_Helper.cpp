#include "IOHandler_Helper.h"
#include "common/def_func.h"
#include <stdexcept>

using namespace std;

// load graph
std::pair<id_t, std::vector<id_t> > IOHelper::load_graph_unweighted(const std::string& line){
	//line: "k\ta b c "
	size_t pos = line.find('\t');
	id_t k = stoid(line.substr(0, pos));
	++pos;

	vector<id_t> data;
	size_t spacepos;
	while((spacepos = line.find(' ',pos)) != line.npos){
		id_t to = stoid(line.substr(pos, spacepos-pos));
		data.push_back(to);
		pos=spacepos+1;
	}
	return make_pair(move(k), move(data));
}

// load graph changes
ChangeEdge<id_t> IOHelper::load_change_unweighted(const std::string& line){
	// <type> is one of A, R
	// line: "<type>\t<src>,<dst>"
	ChangeEdge<id_t> res;
	switch(line[0]){
		case 'A': res.type=ChangeEdgeType::ADD;	break;
		case 'R': res.type=ChangeEdgeType::REMOVE;	break;
		case 'I': res.type=ChangeEdgeType::INCREASE;	break;
		case 'D': res.type=ChangeEdgeType::DECREASE;	break;
		default: throw invalid_argument("Cannot parse change line: "+line);
	}
	size_t p1=line.find(',', 2);
	res.src=stoid(line.substr(2,p1-2));
	size_t p2=line.find(',', p1+1);
	res.dst=stoid(line.substr(p1+1, p2-p1-1));
	return res;
}
