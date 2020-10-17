#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

#include "common.h"

using namespace std;

void dfs(const int p, const int lbl, vector<int>& gid, const vector<vector<int> >& g){
	gid[p]=lbl;
	for(int dst: g[p]){
		if(gid[dst] < lbl){
			dfs(dst, lbl, gid, g);
		}
	}
}

vector<int> cal_cc(const vector<vector<int> >& g) {
	size_t n=g.size();
	// initiate gid as nid
	vector<int> gid;
	gid.reserve(n);
	for(int i=0; i<n; ++i){
		gid.push_back(i);
	}
	
	int ncc=0;
	for(int i=n-1;i>=0;--i){ // start from the largest one
		if(i==gid[i]){
			dfs(i,i,gid,g);
			++ncc;
		}
	}
	cout<<"  # of connected components: "<<ncc<<endl;
	return gid;
}

int main(int argc, char* argv[]){
	if(argc<=3){
		cerr<<"Calculate Connected Component."<<endl;
		cerr<<"Usage: <#parts> <in-folder> <out-folder> [delta-folder]\n"
			<<"  <in-folder>: input file prefix, file name: 'part-<id>' is automatically used\n"
			<<"  <out-folder>: output file prefix, file name 'value-<id>' is automatically used\n"
			<<"  [delta-folder]: (=-) delta file folder, not used by default. File name: 'delta-<id>' is automatically used\n"
			<<endl;
		return 1;
	}
	int parts=stoi(argv[1]);
	string inprefix=argv[2];
	string outprefix=argv[3];
	string deltaprefix;
	if(argc>4)
		deltaprefix=argv[4];
	
	chrono::time_point<std::chrono::system_clock> start_t;
	chrono::duration<double> elapsed;
	
	// load
	cout<<"loading graph"<<endl;
    start_t = chrono::system_clock::now();
	vector<vector<int>> g;
	try{
		g = general_load_unweight(parts, inprefix, "part-", deltaprefix, "delta-");
	}catch(exception& e){
		cerr<<e.what()<<endl;
		return 3;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  load "<<g.size()<<" nodes in "<<elapsed.count()<<" seconds"<<endl;

	// calculate
	cout<<"calculating"<<endl;
	start_t = chrono::system_clock::now();
	vector<int> cc = cal_cc(g);
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	// dump
	cout<<"dumping"<<endl;
	start_t = chrono::system_clock::now();
	if(!general_dump(outprefix, "value-", parts, cc)){
		cerr<<"Error: cannot write to given file(s)"<<endl;
		return 4;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	return 0;
}
