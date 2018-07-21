#include <vector>
#include <string>
#include <utility>

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

