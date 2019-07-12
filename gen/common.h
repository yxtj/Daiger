#pragma once

#include <utility>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

bool beTrueOption(const std::string& str);

struct Link{
	int node;
	float weight;
};
inline bool operator==(const Link &a,const Link &b){
	return a.node == b.node;
}
inline bool operator<(const Link &a,const Link &b){
	return a.weight < b.weight;
}
inline bool operator>(const Link &a,const Link &b){
	return a.weight > b.weight;
}

struct EdgeW{
	int src, dst;
	float weight;
};

struct EdgeUW{
	int src, dst;
};
inline bool operator==(const EdgeUW &a,const EdgeUW &b){
	return a.src == b.src && a.dst == b.src;
}
inline bool operator<(const EdgeUW &a,const EdgeUW &b){
	return a.src == b.src ? a.src < b.src : a.dst < b.dst;
}

std::vector<std::vector<Link>> general_load_weight(const int npart,
	const std::string& gfolder, const std::string& gprefix);

std::vector<std::vector<int>> general_load_unweight(const int npart,
	const std::string& gfolder, const std::string& gprefix);

bool load_graph_weight(std::vector<std::vector<Link>>& res, const std::string& fn);
bool load_graph_unweight(std::vector<std::vector<int>>& res, const std::string& fn);

std::pair<std::unordered_map<int, std::vector<Link>>, size_t> load_graph_weight_one(std::ifstream& fin);
std::pair<std::unordered_map<int, std::vector<int>>, size_t> load_graph_unweight_one(std::ifstream& fin);

std::vector<std::pair<int,int>> load_critical_edges(std::ifstream& fin);

bool dump(const std::vector<std::string>& fnouts, const std::vector<float>& res, const bool fix_point = false);
bool dump(const std::vector<std::string>& fnouts, const std::vector<int>& res, const bool fix_point = false);
bool dump(const std::vector<std::string>& fnouts, const std::vector<std::pair<int, int>>& cedges, const bool fix_point = false);

template <class T>
inline bool general_dump(const std::string& folder, const std::string& prefix,
	const int npart, const std::vector<T>& res, const bool fix_point = false)
{
	std::vector<std::string> fnout;
	for(int i=0;i<npart;++i){
		fnout.push_back(folder+"/"+prefix+std::to_string(i));
	}
	return dump(fnout, res, fix_point);
}

