#include "common.h"
#include <fstream>
#include <algorithm>
#include <iostream>

using namespace std;

bool beTrueOption(const std::string& str){
	static vector<string> true_options({"1", "t", "T", "true", "True", "TRUE", "y", "Y", "yes", "Yes", "YES"});
	return find(true_options.begin(), true_options.end(), str) != true_options.end();
}

std::vector<std::vector<Link>> general_load_weight(const int npart,
	const std::string& gfolder, const std::string& gprefix)
{
	vector<vector<Link>> g;
	for(int i=0;i<npart;++i){
		string fn=gfolder+"/"+gprefix+to_string(i);
		cout<<"  loading "<<fn<<endl;
		if(!load_graph_weight(g, fn)){
			throw invalid_argument("Error: cannot open input file: "+fn);
		}
	}
	return g;
}

std::vector<std::vector<int>> general_load_unweight(const int npart,
	const std::string& gfolder, const std::string& gprefix)
{
	vector<vector<int>> g;
	for(int i=0;i<npart;++i){
		string fn=gfolder+"/"+gprefix+to_string(i);
		cout<<"  loading "<<fn<<endl;
		if(!load_graph_unweight(g, fn)){
			throw invalid_argument("Error: cannot open input file: "+fn);
		}
	}
	return g;
}

bool load_graph_weight(std::vector<std::vector<Link>>& res, const std::string& fn){
	ifstream fin(fn);
	if(!fin){
		return false;
	}
	string line;
	while(getline(fin, line)){
		if(line.size()<2)
			continue;
		size_t pos = line.find('\t');
		int k = stoi(line.substr(0, pos));
		++pos;
		vector<Link> temp;
		size_t spacepos;
		while((spacepos = line.find(' ', pos)) != line.npos){
			size_t cut = line.find(',', pos + 1);
			int node=stoi(line.substr(pos, cut - pos));
			float weight=stof(line.substr(cut + 1, spacepos - cut - 1));
			Link e{node, weight};
			temp.push_back(e);
			pos = spacepos + 1;
		}
		if(res.size() <= k)	// k starts from 0
			res.resize(k+1);	// fill the empty holes
		res[k]=move(temp);
	}
	return true;
}

bool load_graph_unweight(std::vector<std::vector<int>>& res, const std::string& fn){
	ifstream fin(fn);
	if(!fin){
		return false;
	}
	string line;
	while(getline(fin, line)){
		if(line.size()<2)
			continue;
		size_t pos = line.find('\t');
		int k = stoi(line.substr(0, pos));
		++pos;
		vector<int> temp;
		size_t spacepos;
		while((spacepos = line.find(' ', pos)) != line.npos){
			int node=stoi(line.substr(pos, spacepos - pos));
			temp.push_back(node);
			pos = spacepos + 1;
		}
		if(res.size() <= k)	// k starts from 0
			res.resize(k+1);	// fill the empty holes
		res[k]=move(temp);
	}
	return true;
}

std::pair<std::unordered_map<int, std::vector<Link>>, size_t> load_graph_weight_one(std::ifstream& fin){
	unordered_map<int, vector<Link>> res;
	size_t n = 0;
	string line;
	while(getline(fin, line)){
		if(line.size()<2)
			continue;
		size_t pos = line.find('\t');
		int k = stoi(line.substr(0, pos));
		++pos;
		size_t spacepos;
		vector<Link> temp;
		while((spacepos = line.find(' ', pos)) != line.npos){
			size_t cut = line.find(',', pos + 1);
			int node=stoi(line.substr(pos, cut - pos));
			float weight=stof(line.substr(cut + 1, spacepos - cut - 1));
			temp.push_back(Link{node, weight});
			pos = spacepos + 1;
		}
		n += temp.size();
		sort(temp.begin(), temp.end(), [](const Link& a, const Link& b){
			return a.node < b.node;
		});
		res[k] = temp;
	}
	return make_pair(res, n);
}

std::pair<std::unordered_map<int, std::vector<int>>, size_t> load_graph_unweight_one(std::ifstream& fin){
	unordered_map<int, vector<int>> res;
	size_t n = 0;
	string line;
	while(getline(fin, line)){
		if(line.size()<2)
			continue;
		size_t pos = line.find('\t');
		int k = stoi(line.substr(0, pos));
		++pos;
		size_t spacepos;
		vector<int> temp;
		while((spacepos = line.find(' ', pos)) != line.npos){
			int node=stoi(line.substr(pos, spacepos - pos));
			temp.push_back(node);
			pos = spacepos + 1;
		}
		n += temp.size();
		sort(temp.begin(), temp.end());
		res[k] = temp;
	}
	return make_pair(res, n);
}

std::vector<std::pair<int,int>> load_critical_edges(std::ifstream& fin){
	std::vector<std::pair<int,int>> res;
	string line;
	while(getline(fin, line)){
		if(line.size() < 2)
			continue;
		size_t p=line.find(' ');
		int s=stoi(line.substr(0, p));
		int d=stoi(line.substr(p + 1));
		res.emplace_back(s, d);
	}
	sort(res.begin(), res.end());
	return res;
}

bool dump(const std::vector<std::string>& fnouts, const std::vector<float>& res, const bool fix_point){
	size_t parts=fnouts.size();
	vector<ofstream*> fouts;
	for(size_t i=0;i<parts;++i){
		ofstream* pf=new ofstream(fnouts[i]);
		if(!pf || !pf->is_open())
			return false;
		if(fix_point){
			pf->setf(ios::fixed, ios::floatfield);
			pf->precision(6);
		}
		fouts.push_back(pf);
	}
	size_t size=res.size();
	for(size_t i=0;i<size;++i){
		(*fouts[i%parts])<<i<<"\t"<<res[i]<<"\n";
	}
	for(size_t i=0;i<parts;++i)
		delete fouts[i];
	return true;
}

bool dump(const std::vector<std::string>& fnouts, const std::vector<int>& res, const bool){
	size_t parts=fnouts.size();
	vector<ofstream*> fouts;
	for(size_t i=0;i<parts;++i){
		ofstream* pf=new ofstream(fnouts[i]);
		if(!pf || !pf->is_open())
			return false;
		fouts.push_back(pf);
	}
	size_t size=res.size();
	for(size_t i=0;i<size;++i){
		(*fouts[i%parts])<<i<<"\t"<<res[i]<<"\n";
	}
	for(size_t i=0;i<parts;++i)
		delete fouts[i];
	return true;
}

bool dump(const std::vector<std::string>& fnouts, const std::vector<std::pair<int, int>>& cedges, const bool){
	size_t parts=fnouts.size();
	vector<ofstream*> fouts;
	for(size_t i=0;i<parts;++i){
		ofstream* pf=new ofstream(fnouts[i]);
		if(!pf || !pf->is_open())
			return false;
		fouts.push_back(pf);
	}
	size_t size=cedges.size();
	for(size_t i=0;i<size;++i){
		int src, dst;
		tie(src, dst)=cedges[i];
		(*fouts[src%parts])<<src<<" "<<dst<<"\n";
	}
	for(size_t i=0;i<parts;++i)
		delete fouts[i];
	return true;
}
