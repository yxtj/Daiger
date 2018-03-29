#include <vector>
#include <string>

struct Edge{
	int node;
	float weight;
};

bool beTrueOption(const std::string& str);

inline bool operator==(const Edge &a,const Edge &b){
	return a.node == b.node;
}
inline bool operator<(const Edge &a,const Edge &b){
	return a.weight < b.weight;
}
inline bool operator>(const Edge &a,const Edge &b){
	return a.weight > b.weight;
}

std::vector<std::vector<Edge>> general_load_weight(const int npart,
	const std::string& gfolder, const std::string& gprefix, const std::string& dfolder, const std::string& dprefix);

std::vector<std::vector<int>> general_load_unweight(const int npart,
	const std::string& gfolder, const std::string& gprefix, const std::string& dfolder, const std::string& dprefix);

bool load_graph_weight(std::vector<std::vector<Edge>>& res, const std::string& fn);
bool load_graph_unweight(std::vector<std::vector<int>>& res, const std::string& fn);

bool merge_graph_weight(std::vector<std::vector<Edge>>& res, const std::string& fn);
bool merge_graph_unweight(std::vector<std::vector<int>>& res, const std::string& fn);

bool dump(const std::vector<std::string>& fnouts, const std::vector<float>& res);
bool dump(const std::vector<std::string>& fnouts, const std::vector<int>& res);
