#pragma once
#include "def.h"
#include <string>
#include <vector>

namespace common{

// load graph
std::pair<key_t, std::vector<key_t> > load_graph_unweighted(std::string& line);
template <typename W>
std::pair<key_t, std::vector<key_t, W> > load_graph_weighted(std::string& line);

// load graph changes
change_t load_change(std::string& line);

// load starting values
template <typename V>
std::pair<key_t, V> load_value(std::string& line);

}
