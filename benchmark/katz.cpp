#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>

#include "common.h"

using namespace std;

vector<float> cal_katz(const vector<vector<int> >& g, const int source, 
	const float beta, const int maxIter, const double epsilon)
{
	size_t n=g.size();

	vector<float> res(n, 0.0f);
	res[source]=1.0f;
	vector<float> old;
	
	int iter = 0;
	double sum=1.0;
	double oldsum=0;
	while(++iter < maxIter && abs(sum-oldsum) > epsilon){
		old.swap(res); // old=move(res);
		oldsum=sum;
		res.assign(n, 0.0f);
		// sum=n*static_cast<double>(1-beta);
		sum = 0.0;
		for(size_t i=0;i<n;++i){
			const vector<int>& line=g[i];
			float out=beta*old[i];
			for(int dst : line){
				res[dst]+=out;
			}
			//sum+=beta*old[i];
		}
		res[source]=1.0f;
		for(auto& v : res)
			sum += v*v;
	}
	cout<<"  iterations: "<<iter<<"\tdifference: "<<sum-oldsum<<endl;
	return res;
}

vector<float> cal_katz_dlt(const vector<vector<int> >& g, const int source, 
	const float beta, const int maxIter, const double epsilon)
{
	size_t n=g.size();

	vector<float> res(n, 0);
	vector<float> dlt_old(n);
	vector<float> dlt_new(n, 0);
	dlt_new[source]=1.0f;
	
	int iter = 0;
	double sum=n*static_cast<double>((1-beta)*(1-beta));
	while(++iter < maxIter && sum > epsilon){
		dlt_old.swap(dlt_new);
		dlt_new.assign(n, 0);
		sum = 0.0;
		for(size_t i=0;i<n;++i){
			const vector<int>& line=g[i];
			float out=beta*dlt_old[i];
			for(int dst : line){
				dlt_new[dst]+=out;
			}
		}
		//cout<<"iteration: "<<iter<<"\n";
		for(size_t i=0;i<n;++i){
			//cout<<" "<<i<<": "<<res[i]<<"\t"<<dlt_old[i]<<"\n";
			res[i] += dlt_old[i];
			sum += dlt_new[i] * dlt_new[i];
		}
	}
	cout<<"  iterations: "<<iter<<"\tdifference: "<<sum<<endl;
	return res;
}

int main(int argc, char* argv[]){
	if(argc<=3){
		cerr<<"Calculate Katz distance."<<endl;
		cerr<<"Usage: <#parts> <in-folder> <out-folder> [delta-folder] [source] [beta] [max-iter] [epsilon] [method]\n"
			<<"  <in-folder>: input file folder, file name: 'part-<id>' is automatically used\n"
			<<"  <out-folder>: output file folder, file name 'value-<id>' is automatically used\n"
			<<"  [delta-folder]: (=-) delta file folder, not used by default. File name: 'delta-<id>' is automatically used\n"
			<<"  [source]: (=0) the source node to calculate katz distance\n"
			<<"  [beta]: (=0.2) the decay factor (the portion of values transitted)\n"
			<<"  [max-iter]: (=inf) the maximum number of iterations until termination\n"
			<<"  [epsilon]: (=1e-6) the minimum difference between consecutive iterations for termination check\n"
			<<"  [method]: (=normal) the method for calculation. Supports: normal, delta"
			<<endl;
		return 1;
	}
	int parts=stoi(argv[1]);
	string inprefix=argv[2];
	string outprefix=argv[3];
	string deltaprefix;
	if(argc>4)
		deltaprefix=argv[4];
	int source=0;
	if(argc>5)
		source=stoi(argv[5]);
	float beta=0.8;
	if(argc>6)
		beta=stof(argv[6]);
	int maxIter=numeric_limits<int>::max();
	if(argc>7){
		if(argv[7] == "inf")
			maxIter=numeric_limits<int>::max();
		else
			maxIter=stoi(argv[7]);
	}
	double termDiff=1e-6;
	if(argc>8)
		termDiff=stod(argv[8]);
	string method="normal";
	if(argc>9)
		method=argv[9];
	
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
	vector<float> res;
	if(method == "normal"){
		res = cal_katz(g, source, beta, maxIter, termDiff);
	}else if(method == "delta"){
		res = cal_katz_dlt(g, source, beta, maxIter, termDiff);
	}else{
		cerr<<"method not supported"<<endl;
		return 4;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	// dump
	cout<<"dumping"<<endl;
	start_t = chrono::system_clock::now();
	if(!general_dump(outprefix, "value-", parts, res, true)){
		cerr<<"Error: cannot write to given file(s)"<<endl;
		return 5;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	return 0;
}
