#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <numeric> // accumulate
#include <random>
#include <chrono>

#include "common.h"

using namespace std;

void normalize_trans_prob(vector<vector<Edge>>& g){
	auto fun = [](const float s, const Edge& e){
		return s+e.weight;
	};
	for(auto& line : g){
		if(line.empty())
			continue;
		float s = accumulate(line.begin(), line.end(), 0.0f, fun);
		for(auto& e : line){
			e.weight /= s;
		}
	}
}

vector<float> init_dis_fix(const size_t n, const float value, const bool normalize){
	float v = normalize ? value/n : value;
	return vector<float>(n, v);
}
vector<float> init_dis_rand(const size_t n, const unsigned long seed, const bool normalize){
	mt19937 gen(seed);
	uniform_real_distribution<float> dis(0.0f, 1.0f);
	vector<float> res;
	res.reserve(n);
	double s = 0.0;
	for(size_t i = 0; i < n; ++i){
		float v = dis(gen);
		s+=v;
		res.push_back(v);
	}
	if(normalize){
		for(auto& v : res)
			v /= s;
	}
	return res;
}

vector<float> init_dis(const size_t n, const string& initm, const bool normalize){
	size_t p = initm.find(':');
	if(p == 3){ // fix:<v>
		float v = stof(initm.substr(p+1));
		return init_dis_fix(n, v, normalize);
	}else{ // rand:<s>
		unsigned long s = stoul(initm.substr(p+1));
		return init_dis_rand(n, s, normalize);
	}	
}

double square_sum(const double a, const float b){
	return a+b*b;
}

vector<float> cal_mc(vector<vector<Edge>>& g, const string& initm,
	const bool normalize, const int maxIter, const double epsilon)
{
	size_t n=g.size();
	normalize_trans_prob(g);

	vector<float> res = init_dis(n, initm, normalize);
	vector<float> old(n, 0.0f);
	
	int iter = 0;
	double sum=accumulate(res.begin(), res.end(), 0.0, square_sum);
	double oldsum=0;
	while(++iter < maxIter && abs(sum-oldsum) > epsilon){
		old=move(res);
		res.clear();
		res.resize(n, 0.0f);
		oldsum=sum;
		sum = 0.0;
		for(size_t i=0;i<n;++i){
			float v=old[i];
			const vector<Edge>& line=g[i];
			for(const Edge& e : line){
				res[e.node]+=v*e.weight;
			}
		}
		sum=accumulate(res.begin(), res.end(), 0.0, square_sum);
	}
	cout<<"  iterations: "<<iter<<"\tdifference: "<<sum-oldsum<<endl;
	return res;
}

bool check_init_method(const string& initm){
	size_t p = initm.find(':');
	if(p==string::npos)
		return false;
	string m=initm.substr(0, p);
	if(p == 3 && m == "fix"){
		try{
			float v = stof(initm.substr(p+1));
		}catch(...){
			return false;
		}
	}else if(p == 4 && m == "rand"){
		try{
			unsigned long s = stoul(initm.substr(p+1));
		}catch(...){
			return false;
		}
	}else{
		return false;
	}
	return true;
}

int main(int argc, char* argv[]){
	if(argc<=3){
		cerr<<"Calculate Markov Chain stational distribution."<<endl;
		cerr<<"Usage: <#parts> <in-folder> <out-folder> [delta-folder] [init-method] [normalize] [max-iter] [epsilon]\n"
			<<"  <in-folder>: input file prefix, file name: 'part-<id>' is automatically used\n"
			<<"  <out-folder>: output file prefix, file name 'value-<id>' is automatically used\n"
			<<"  [delta-folder]: (=-) delta file folder, not used by default. File name: 'delta-<id>' is automatically used\n"
			<<"  [init-method]: (=fix:1) method of setting initialization value. "
				"Support: fix:<v> and rand:<s>, meaning fixed at <v> (float) or randomly set using seed <s> (int)\n"
			<<"  [normalize]: (=0) whether to normalize the result\n"
			<<"  [max-iter]: (=inf) the maximum number of iterations until termination\n"
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
	string initm="fix:1";
	if(argc>5)
		initm=argv[5];
	bool normalize = false;
	if(argc>6)
		normalize = beTrueOption(argv[6]);
	int maxIter=numeric_limits<int>::max();
	if(argc>7)
		maxIter=stoi(argv[7]);
	double termDiff=1e-6;
	if(argc>8)
		termDiff=stod(argv[8]);
	
	if(!check_init_method(initm)){
		cerr<<"the initialization method is not correct"<<endl;
		return 2;
	}
		
	chrono::time_point<std::chrono::system_clock> start_t;
	chrono::duration<double> elapsed;
	
	// load
	cout<<"loading graph"<<endl;
    start_t = chrono::system_clock::now();
	vector<vector<Edge>> g;
	try{
		g = general_load_weight(parts, inprefix, "part-", deltaprefix, "delta-");
	}catch(exception& e){
		cerr<<e.what()<<endl;
		return 3;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  load "<<g.size()<<" nodes in "<<elapsed.count()<<" seconds"<<endl;
	
	// calculate
	cout<<"calculating Markov Chain"<<endl;
	start_t = chrono::system_clock::now();
	vector<float> res = cal_mc(g, initm, normalize, maxIter, termDiff);
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	// dump
	cout<<"dumping Markov Chain"<<endl;
	start_t = chrono::system_clock::now();
	if(!general_dump(outprefix, "value-", parts, res)){
		cerr<<"Error: cannot write to given file(s)"<<endl;
		return 4;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	return 0;
}
