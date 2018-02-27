#pragma once
#include "def.h"
#include <string>
#include <vector>

namespace common{

// load graph
// format: "k\ta b c "
std::pair<key_t, std::vector<key_t> > load_graph_unweighted(std::string& line);
// format: "k\tai,aw bi,bw ci,cw "
template <typename W>
std::pair<key_t, std::vector<key_t, W> > load_graph_weighted(std::string& line);

// load graph changes
// format: <type> is one of A, R, I, D
// for A and R:
// line: "<type>\t<src>,<dst>"
// for I and D:
// line: "<type>\t<src>,<dst>,<weight>"
change_t load_change(std::string& line);

// load starting values
// format: "<key>\t<value>"
template <typename V>
std::pair<key_t, V> load_value(std::string& line);

// dump result values
// format: "<key>\t<value>"
template <typename V>
std::string dump_value(const key_t& k, const V& v);

}
