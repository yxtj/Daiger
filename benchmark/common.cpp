#include "common.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace std;

bool beTrueOption(const std::string& str){
	static vector<string> true_options({"1", "t", "T", "true", "True", "TRUE", "y", "Y", "yes", "Yes", "YES"});
	return find(true_options.begin(), true_options.end(), str) != true_options.end();
}

std::vector<std::vector<Edge>> general_load_weight(const int npart,
	const std::string& gfolder, const std::string& gprefix, const std::string& dfolder, const std::string& dprefix)
{
	vector<vector<Edge>> g;
	for(int i=0;i<npart;++i){
		string fn=gfolder+"/"+gprefix+to_string(i);
		cout<<"  loading "<<fn<<endl;
		if(!load_graph_weight(g, fn)){
			throw invalid_argument("Error: cannot open input file: "+fn);
		}
	}
	if(!dfolder.empty() && dfolder != "-"){
		cout<<"loading delta"<<endl;
		for(int i=0;i<npart;++i){
			string fn=dfolder+"/"+dprefix+to_string(i);
			cout<<"  loading "<<fn<<endl;
			if(!merge_graph_weight(g, fn)){
				throw invalid_argument("Error: cannot open delta file: "+fn);
			}
		}
	}
	return g;
}

std::vector<std::vector<int>> general_load_unweight(const int npart,
	const std::string& gfolder, const std::string& gprefix, const std::string& dfolder, const std::string& dprefix)
{
	vector<vector<int>> g;
	for(int i=0;i<npart;++i){
		string fn=gfolder+"/"+gprefix+to_string(i);
		cout<<"  loading "<<fn<<endl;
		if(!load_graph_unweight(g, fn)){
			throw invalid_argument("Error: cannot open input file: "+fn);
		}
	}
	if(!dfolder.empty() && dfolder != "-"){
		cout<<"loading delta"<<endl;
		for(int i=0;i<npart;++i){
			string fn=dfolder+"/"+dprefix+to_string(i);
			cout<<"  loading "<<fn<<endl;
			if(!merge_graph_unweight(g, fn)){
				throw invalid_argument("Error: cannot open delta file: "+fn);
			}
		}
	}
	return g;
}


bool load_graph_weight(std::vector<std::vector<Edge>>& res, const std::string& fn){
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
		vector<Edge> temp;
		size_t spacepos;
		while((spacepos = line.find(' ', pos)) != line.npos){
			size_t cut = line.find(',', pos + 1);
			int node=stoi(line.substr(pos, cut - pos));
			float weight=stof(line.substr(cut + 1, spacepos - cut - 1));
			Edge e{node, weight};
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
		if(res.size() < k)	// k starts from 0
			res.resize(k);	// fill the empty holes
		res.push_back(move(temp));
	}
	return true;
}

bool merge_graph_weight(std::vector<std::vector<Edge>>& res, const std::string& fn){
	ifstream fin(fn);
	if(!fin){
		return false;
	}
	string line;
	while(getline(fin, line)){
		if(line.size()<2)
			continue;
		char type = line[0];
		size_t p1 = line.find(',', 2);
		int src = stoi(line.substr(2, p1));
		size_t p2 = line.find(',', p1+1);
		int dst = stoi(line.substr(p1+1, p2));
		auto& vec = res[src];
		auto it = lower_bound(vec.begin(), vec.end(), dst, [](const Edge& e, int dst){
			return e.node < dst;
		});
		if(type == 'R'){
			if(it != vec.end())
				vec.erase(it);
			continue;
		}
		float v = stof(line.substr(p2+1));
		if(type == 'A'){
			if(it != vec.end() && it->node == dst)
				it->weight = v;
			else
				vec.insert(it, Edge{dst, v});
		}else{ // type == 'I' || type == 'D'
			if(it->node == dst)
				it->weight = v;
			else
				vec.insert(it, Edge{dst, v});
		}
	}
	return true;
}

bool merge_graph_unweight(std::vector<std::vector<int>>& res, const std::string& fn){
	ifstream fin(fn);
	if(!fin){
		return false;
	}
	string line;
	while(getline(fin, line)){
		if(line.size()<2)
			continue;
		char type = line[0];
		size_t pos = line.find(',', 2);
		int src = stoi(line.substr(2, pos));
		int dst = stoi(line.substr(pos+1));
		if(type == 'A'){
			auto& vec = res[src];
			auto it = lower_bound(vec.begin(), vec.end(), dst);
			if(it != vec.end() && *it == dst)
				continue;
			vec.insert(it, dst);
		}else{ // type == 'R'
			auto& vec = res[src];
			auto it = lower_bound(vec.begin(), vec.end(), dst);
			if(it == vec.end())
				continue;
			vec.erase(it);
		}
		
	}
	return true;
}

bool dump(const std::vector<std::string>& fnouts, const std::vector<float>& res){
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

bool dump(const std::vector<std::string>& fnouts, const std::vector<int>& res){
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


