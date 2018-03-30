#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>

#include "common.h"

using namespace std;

vector<float> cal_pr(const vector<vector<int> >& g, const float damp,
	const bool normalize, const int maxIter, const double epsilon)
{
	size_t n=g.size();

	vector<float> res(n, 1-damp);
	vector<float> old;
	
	int iter = 0;
	double sum=n*static_cast<double>(1-damp);
	double oldsum=0;
	while(++iter < maxIter && abs(sum-oldsum) > epsilon){
		old.swap(res); // old=move(res);
		oldsum=sum;
		res.assign(n, 1-damp);
		// sum=n*static_cast<double>(1-damp);
		sum = 0.0;
		for(size_t i=0;i<n;++i){
			const vector<int>& line=g[i];
			float out=damp*old[i]/line.size();
			for(int dst : line){
				res[dst]+=out;
			}
			//sum+=damp*old[i];
		}
		for(auto& v : res)
			sum += v*v;
	}
	if(normalize){
		double s = 0.0;
		for(auto& v : res)
			s += v;
		for(auto& v : res)
			v /= s;
	}
	cout<<"  iterations: "<<iter<<"\tdifference: "<<sum-oldsum<<endl;
	return res;
}

int main(int argc, char* argv[]){
	if(argc<=3){
		cerr<<"Calculate PageRank."<<endl;
		cerr<<"Usage: <#parts> <in-folder> <out-folder> [delta-folder] [dump-factor] [normalize] [max-iter] [epsilon]\n"
			<<"  <in-folder>: input file folder, file name: 'part-<id>' is automatically used\n"
			<<"  <out-folder>: output file folder, file name 'value-<id>' is automatically used\n"
			<<"  [delta-folder]: (=-) delta file folder, not used by default. File name: 'delta-<id>' is automatically used\n"
			<<"  [damp-factor]: (=0.8) the damping factor (the portion of values transitted) for PageRank\n"
			<<"  [max-iter]: (=inf) the maximum number of iterations until termination\n"
			<<"  [normalize]: (=0) whether to normalize the result\n"
			<<"  [epsilon]: (=1e-6) the minimum difference between consecutive iterations for termination check"
			<<endl;
		return 1;
	}
	int parts=stoi(argv[1]);
	string inprefix=argv[2];
	string outprefix=argv[3];
	string deltaprefix;
	if(argc>4)
		deltaprefix=argv[4];
	float damp=0.8;
	if(argc>5)
		damp=stof(argv[5]);
	bool normalize = false;
	if(argc>6)
		normalize = beTrueOption(argv[6]);
	int maxIter=numeric_limits<int>::max();
	if(argc>7)
		maxIter=stoi(argv[7]);
	double termDiff=1e-6;
	if(argc>8)
		termDiff=stod(argv[8]);
	
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
	vector<float> res = cal_pr(g, damp, normalize, maxIter, termDiff);
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	// dump
	cout<<"dumping"<<endl;
	start_t = chrono::system_clock::now();
	vector<string> fnout;
	for(int i=0;i<parts;++i){
		fnout.push_back(outprefix+"/value-"+to_string(i));
	}
	if(!dump(fnout, res)){
		cerr<<"Error: cannot write to given file(s)"<<endl;
		return 4;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	return 0;
}
