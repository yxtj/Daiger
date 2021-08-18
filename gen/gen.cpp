/*
 * main.cpp
 *
 *  Created on: Jan 11, 2016
 *      Author: tzhou
 *  Modified on Mar 17, 2017
 *      Add weight and more options
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <random>
#include <functional>
#include "common.h"
#include "pl-dis.hpp"

using namespace std;

template <typename T>
pair<size_t, size_t> getGraphInfo(const vector<vector<T>>& g){
	size_t nn = g.size();
	size_t ne = 0;
	for(const auto& l: g){
		ne += l.size();
	}
	return make_pair(nn, ne);
}

// ------ online unweighted ------

vector<vector<int> > gen_one_uw(const int nPart, const int id, const int nNode,
		mt19937& gen, function<unsigned(mt19937&)> rngDeg, const bool noSelfLoop)
{
	vector<vector<int> > g(nNode);
	auto deg_gen=bind(rngDeg,gen);
	uniform_int_distribution<unsigned> n_dis(0,nNode-1);
	auto dst_gen=[&](){ return n_dis(gen); };
	for(int i=id;i<nNode;i+=nPart){
		int m=deg_gen();
		for(int j=0;j<m;++j){
			g[i].push_back(dst_gen());
		}
		sort(g[i].begin(),g[i].end());
		g[i].erase(unique(g[i].begin(),g[i].end()),g[i].end());
		if(noSelfLoop){
			auto it=lower_bound(g[i].begin(), g[i].end(), i);
			if(it!=g[i].end() && *it==i)
				g[i].erase(it);
		}
	}
	return g;
}

bool dump_one_uw(const vector<vector<int> >& g, const int nPart, const int id, const string& outDir){
	ofstream fout(outDir+"/part-"+to_string(id));
	if(!fout.is_open())
		return false;
	for(size_t i=id;i<g.size();i+=nPart){
		fout<<i<<"\t";
		for(int v : g[i])
			fout<<v<<" ";
		fout<<"\n";
	}
	return true;
}

// ------ online weighted ------

vector<vector<pair<int,double>> > gen_one_w(const int nPart, const int id, const int nNode, mt19937& gen,
	function<unsigned(mt19937&)> rngDeg, function<double(mt19937&)> rngWgt, const bool noSelfLoop)
{
	vector<vector<pair<int,double>> > g(nNode);
	auto deg_gen=bind(rngDeg,gen);
	uniform_int_distribution<unsigned> n_dis(0,nNode-1);
	auto dst_gen=[&](){ return n_dis(gen); };
	auto wgt_gen=bind(rngWgt,gen);
	for(int i=id;i<nNode;i+=nPart){
		int m=deg_gen();
		for(int j=0;j<m;++j){
			g[i].emplace_back(dst_gen(),wgt_gen());
		}
		sort(g[i].begin(),g[i].end(),[](const pair<int,double>& lth, const pair<int,double>& rth){
			return lth.first<rth.first;
		});
		auto it=unique(g[i].begin(),g[i].end(),[](const pair<int,double>& lth, const pair<int,double>& rth){
			return lth.first==rth.first;
		});
		g[i].erase(it,g[i].end());
		if(noSelfLoop){
			auto it=lower_bound(g[i].begin(), g[i].end(), i, [](const pair<int,double>& lth, int rth){
				return lth.first<rth;
			});
			if(it!=g[i].end() && it->first==i)
				g[i].erase(it);
		}
	}
	return g;
}

bool dump_one_w(const vector<vector<pair<int,double>> >& g, const int nPart, const int id, const string& outDir){
	ofstream fout(outDir+"/part-"+to_string(id));
	if(!fout.is_open())
		return false;
	for(size_t i=id;i<g.size();i+=nPart){
		fout<<i<<"\t";
		for(pair<int,double> vw : g[i])
			fout<<vw.first<<","<<vw.second<<" ";
		fout<<"\n";
	}
	return true;
}

// ------ offline unweighted ------

vector<vector<int> > gen_uw(const int nPart, const int nNode, const unsigned long seed,
	function<unsigned(mt19937&)> rngDeg, bool directed, bool noSelfLoop)
{
	vector<vector<int> > g(nNode);
	mt19937 gen(seed);
	auto deg_gen=bind(rngDeg,gen);
	uniform_int_distribution<unsigned> n_dis(0,nNode-1);
	auto dst_gen=[&](){ return n_dis(gen); };
	if(directed){
		for(int i=0;i<nNode;++i){
			int m=deg_gen();
			for(int j=0;j<m;++j){
				g[i].push_back(dst_gen());
			}
		}
	}else{
		for(int i=0;i<nNode;++i){
			int m=deg_gen();
			for(int j=0;j<m;++j){
				int d = dst_gen();
				g[i].push_back(d);
				g[d].push_back(i);
			}
		}
	}
	for(int i=0;i<nNode;++i){
		sort(g[i].begin(),g[i].end());
		g[i].erase(unique(g[i].begin(),g[i].end()),g[i].end());
		if(noSelfLoop){
			auto it=lower_bound(g[i].begin(), g[i].end(), i);
			if(it!=g[i].end() && *it==i)
				g[i].erase(it);
		}
	}
	return g;
}

int dump_uw(const vector<vector<int> >& g, const int nPart, const string& outDir){
	mkdir(outDir.c_str(),0755);
	vector<ofstream*> fout;
	for(int i=0;i<nPart;++i){
		fout.push_back(new ofstream(outDir+"/part-"+to_string(i)));
		if(!fout.back()->is_open()){
			cerr<<"failed in opening file: "<<outDir+"/part-"+to_string(i)<<endl;
			return i;
		}
	}
	for(size_t i=0;i<g.size();++i){
		int idx=i%nPart;
		*fout[idx]<<i<<"\t";
		for(int v : g[i])
			*fout[idx]<<v<<" ";
		*fout[idx]<<"\n";
	}
	for(size_t i=0;i<fout.size();++i){
		delete fout[i];
	}
	return nPart;
}

// ------ offline weighted ------

vector<vector<pair<int,double>> > gen_w(const int nPart, const int nNode, const unsigned long seed,
	function<unsigned(mt19937&)> rngDeg, function<double(mt19937&)> rngWgt, bool directed, bool noSelfLoop)
{
	vector<vector<pair<int,double>> > g(nNode);
	mt19937 gen(seed);
	auto deg_gen=bind(rngDeg,gen);
	uniform_int_distribution<unsigned> n_dis(0,nNode-1);
	auto dst_gen=[&](){ return n_dis(gen); };
	auto wgt_gen=bind(rngWgt,gen);
	if(directed){
		for(int i=0;i<nNode;++i){
			int m=deg_gen();
			for(int j=0;j<m;++j){
				g[i].emplace_back(dst_gen(),wgt_gen());
			}
		}
	}else{
		for(int i=0;i<nNode;++i){
			int m=deg_gen();
			for(int j=0;j<m;++j){
				int d=dst_gen();
				double w=wgt_gen();
				g[i].emplace_back(d,w);
				g[d].emplace_back(i,w);
			}
		}
		
	}
	for(int i=0;i<nNode;++i){
		sort(g[i].begin(),g[i].end(),[](const pair<int,double>& lth, const pair<int,double>& rth){
			return lth.first<rth.first;
		});
		auto it=unique(g[i].begin(),g[i].end(),[](const pair<int,double>& lth, const pair<int,double>& rth){
			return lth.first==rth.first;
		});
		g[i].erase(it,g[i].end());
		if(noSelfLoop){
			auto it=lower_bound(g[i].begin(), g[i].end(), i, [](const pair<int,double>& lth, int rth){
				return lth.first<rth;
			});
			if(it!=g[i].end() && it->first==i)
				g[i].erase(it);
		}
	}
	return g;
}

int dump_w(const vector<vector<pair<int,double>> >& g, const int nPart, const string& outDir){
	mkdir(outDir.c_str(),0755);
	vector<ofstream*> fout;
	for(int i=0;i<nPart;++i){
		fout.push_back(new ofstream(outDir+"/part-"+to_string(i)));
		if(!fout.back()->is_open()){
			cerr<<"failed in opening file: "<<outDir+"/part-"+to_string(i)<<endl;
			return i;
		}
	}
	for(size_t i=0;i<g.size();++i){
		int idx=i%nPart;
		*fout[idx]<<i<<"\t";
		for(pair<int,double> vw : g[i])
			*fout[idx]<<vw.first<<","<<vw.second<<" ";
		*fout[idx]<<"\n";
	}
	for(size_t i=0;i<fout.size();++i){
		delete fout[i];
	}
	return nPart;
}

// ------ main ------

struct Option{
	int nPart, nNode;
	string dist;
	vector<string> distParam;
	string weight;
	double wmin,wmax;
	bool directed;
	string outDir;
	bool noSelfLoop;
	bool online;
	unsigned long seed;

	void parse(int argc, char* argv[]);
private:
	bool setDist(string& method);
	bool setWeight(string& method);
};
void Option::parse(int argc, char* argv[]){
	nPart=stoi(string(argv[1]));
	nNode=stoi(string(argv[2]));
	outDir=argv[3];
	directed=true;
	if(argc>4)
		directed=beTrueOption(string(argv[4]));
	noSelfLoop=false;
	if(argc>5){
		noSelfLoop=beTrueOption(string(argv[5]));
	}
	string weightMethod="no";
	if(argc>6)
		weightMethod=argv[6];
	string distMethod="pl:2.3";
	if(argc>7)
		distMethod=argv[7];
	online = false;
	if(argc > 8)
		online = beTrueOption(string(argv[8]));
	seed=1535345;
	if(argc>9)
		seed=stoul(string(argv[9]));
	// check
	if(online && !directed)
		throw invalid_argument("do not support undirected graph in online mode");
	// check distribution
	if(!setDist(distMethod))
		throw invalid_argument("unsupported degree distribution: "+distMethod);
	if(!setWeight(weightMethod))
		throw invalid_argument("unsupported weight distribution: "+weightMethod);
}
bool Option::setDist(string& method){
	auto p=method.find(":");
	if(p!=string::npos){
		dist=method.substr(0,p);
		distParam.clear();
		++p;
		size_t pp;
		while((pp = method.find(',',p)) != method.npos){
			string to = method.substr(p, pp-p);
			distParam.push_back(to);
			p=pp+1;
		}
		if(p<method.size()){
			distParam.push_back(method.substr(p));
		}
	}else{
		return false;
	}
	vector<string> supported = {"uni","pl"};
	return find(supported.begin(), supported.end(), dist)!=supported.end();
}
bool Option::setWeight(string& method){
	if(method=="no"){
		weight="no";
	}else if(method.substr(0,7)=="weight:"){
		weight="weight";
		size_t p=method.find(',',7);
		wmin=stod(method.substr(7,p-7));
		wmax=stod(method.substr(p+1));
	}else{
		return false;
	}
	return true;
}

int main(int argc, char* argv[]){
	if(argc<3 || argc>10){
		cerr<<"Generate graph.\n"
			"Usage: <#parts> <#nodes> <out-fld> [directed] [no-self-loop] [weight:<min>,<max>] [deg-dist:<param>] [online] [random-seed]"<<endl;
		cerr<<"  <out-fld>: the folder to store the output fieles\n"
			"  [directed]: (=1) whether to generate directed graph\n"
			"  [no-self-loop]: (=0) whether to remove self loop edges\n"
			"  [weight]: (=no) the weight distribution. If unweighted graph is needed, use \"no\" here.\n"
			"  [deg-dist]: (=pl:2.3) the degree distribution. Support: uni:<min>,<max> (uniform in range [min,max]), pl:<alpha> (power-law with alpha)\n"
			"  [online]: (=0) whether to perform online generation. "
			"Offline version guarantees equivalent output for same graph and seed. "
			"Online version guarantees that ONLY when <#part> is also identical (optimized for huge graph).\n"
			"  [random-seed]: (=1535345) seed for random numbers.\n"
			"i.e.: ./gen.exe 2 100 out 1 0 no pl:2.6 0 123456 \n"
			"i.e.: ./gen.exe 2 100 out 1 0 weight:0,1 uni \n"<<endl;
		return 1;
	}
	Option opt;
	try{
		opt.parse(argc, argv);
	}catch(exception& e){
		cerr<<e.what()<<endl;
		return 2;
	}
	ios_base::sync_with_stdio(false);
	cout<<"generating with "<<opt.nNode<<" nodes, "<<opt.nPart<<" parts, in folder: "<<opt.outDir<<endl;
	cout<<"directed: "<<opt.directed<<", remove self-loop: "<<opt.noSelfLoop<<endl;
	cout<<"degree distribution: "<<opt.dist<<", random seed: "<<opt.seed<<endl;

	function<unsigned(mt19937&)> rngDeg;
	uniform_int_distribution<int> uni_dis(0,opt.nNode-1);
	power_law_distribution<unsigned> pl_dis(1, 2.3);
	if(opt.dist=="uni"){
		uni_dis=uniform_int_distribution<int>(stoi(opt.distParam[0]), stoi(opt.distParam[1]));
		rngDeg=bind(uni_dis,placeholders::_1);
	}else{
		pl_dis=power_law_distribution<unsigned>(1, stod(opt.distParam[0]));
		rngDeg=[&](mt19937& m){ return min<unsigned>(pl_dis(m), opt.nNode); };
	}
	
	mkdir(opt.outDir.c_str(),0755);

	int n;
	if(opt.online){ // online
		mt19937 gen(opt.seed);
		if(opt.weight=="no"){
			for(int i=0; i<opt.nPart; ++i){
				cout<<"  "<<i+1<<"/"<<opt.nPart<<endl;
				vector<vector<int> > g=gen_one_uw(opt.nPart,i,opt.nNode,gen,rngDeg,opt.noSelfLoop);
				pair<size_t, size_t> cnt=getGraphInfo(g);
				cout<<"  generate "<<cnt.first<<" nodes and "<<cnt.second<<" edges"<<endl;
				bool f=dump_one_uw(g,opt.nPart,i,opt.outDir);
				if(f)
					++n;
			}
		}else{
			uniform_real_distribution<double> uni_dis(opt.wmin, opt.wmax);
			function<double(mt19937&)> rngWgt=bind(uni_dis,placeholders::_1);
			for(int i=0; i<opt.nPart; ++i){
				cout<<"  "<<i+1<<"/"<<opt.nPart<<endl;
				vector<vector<pair<int,double>> > g=gen_one_w(opt.nPart,i,opt.nNode,gen,rngDeg,rngWgt,opt.noSelfLoop);
				pair<size_t, size_t> cnt=getGraphInfo(g);
				cout<<"  generate "<<cnt.first<<" nodes and "<<cnt.second<<" edges"<<endl;
				bool f=dump_one_w(g,opt.nPart,i,opt.outDir);
				if(f)
					++n;
			}
		}
	}else{ // offline
		if(opt.weight=="no"){
			vector<vector<int> > g=gen_uw(opt.nPart,opt.nNode,opt.seed,rngDeg,opt.directed,opt.noSelfLoop);
			pair<size_t, size_t> cnt=getGraphInfo(g);
			cout<<"generate "<<cnt.first<<" nodes and "<<cnt.second<<" edges"<<endl;
			cout<<"dumping"<<endl;
			n=dump_uw(g,opt.nPart,opt.outDir);
		}else{
			uniform_real_distribution<double> uni_dis(opt.wmin, opt.wmax);
			function<double(mt19937&)> rngWgt=bind(uni_dis,placeholders::_1);
			vector<vector<pair<int,double>> > g=gen_w(opt.nPart,opt.nNode,opt.seed,rngDeg,rngWgt,opt.directed,opt.noSelfLoop);
			pair<size_t, size_t> cnt=getGraphInfo(g);
			cout<<"generate "<<cnt.first<<" nodes and "<<cnt.second<<" edges"<<endl;
			cout<<"dumping"<<endl;
			n=dump_w(g,opt.nPart,opt.outDir);
		}
	}
	cout<<"success "<<n<<" files. fail "<<opt.nPart-n<<" files."<<endl;
	return 0;
}

