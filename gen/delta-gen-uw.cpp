/*
 * deltaGen.cpp
 *
 *  Created on: Jan 11, 2016
 *      Author: tzhou
 *  Modified on Mar 17, 2017
 *      Add weight and more options
 *  Modified on April 23, 2017 by GZ
 *		generate delta graph files
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <functional>
#include <unordered_set>
#include <chrono>
#include "common.h"

using namespace std;

// ---- load the graph data and generate the delta file

struct Edge{
	int u, v;
};

struct ModifyThreshold{
	double trivial;
	double add;
	double rmv;
};

struct ModifyEdges{
	vector<Edge> addSet;
	vector<Edge> rmvSet;
};

size_t addReverseEdge(vector<Edge>& es, const bool doSort){
	vector<pair<int, int>> tmp;
	tmp.reserve(es.size());
	// find those already have a reverse edge in es
	for(Edge& e : es){
		if(e.u < e.v)
			tmp.emplace_back(e.u, e.v);
		else
			tmp.emplace_back(e.v, e.u);
	}
	sort(tmp.begin(), tmp.end());
	vector<pair<int, int>> ext;
	auto it = adjacent_find(tmp.begin(), tmp.end());
	while(it != tmp.end()){
		ext.push_back(*it);
		++it;
		it = adjacent_find(it, tmp.end());
	}
	// add reverse edges
	size_t n = es.size();
	es.reserve(2*n);
	if(ext.empty()){
		for(size_t i = 0; i<n; ++i){
			const auto& e = es[i];
			es.push_back(Edge{e.v, e.u});
		}
	}else{
		for(size_t i = 0; i<n; ++i){
			const auto& e = es[i];
			pair<int, int> t = e.u < e.v ? make_pair(e.u, e.v) : make_pair(e.v, e.u);
			if(find(ext.begin(), ext.end(), t) == ext.end())
				es.push_back(Edge{e.v, e.u});
		}
	}
	if(doSort){
		sort(es.begin(), es.end(), [](const Edge& l, const Edge& r){
			return l.u < r.u ? true : l.u == r.u && l.v < r.v;
		});
	}
	return (es.size() + ext.size()) / 2;
}

void dumpChangeOneSet(vector<ofstream*>& fouts, const int n, const vector<Edge>& es, char type){
	for(const Edge& e : es){
		(*fouts[e.u % n]) << type << " " << e.u << "," << e.v << "\n";
	}
}

// ------ online ------

pair<int, vector<Edge>> parseFromLine(const string& line){
	int key;
	vector<Edge> data;
	size_t pos = line.find('\t');
	key = stoi(line.substr(0, pos));
	++pos;

	size_t spacepos;
	while((spacepos = line.find(' ', pos)) != line.npos){
		int node = stoi(line.substr(pos, spacepos - pos));
		Edge e{ key, node };
		data.push_back(e);
		pos = spacepos + 1;
	}
	return make_pair(key, data);
}

// normal return: (totalV, totalE, maxV)
// return by reference: resultSet = {addSet, rmvSet, incSet, decSet}
tuple<int, int, int> changeOne(ifstream& fin, int maxV, const ModifyThreshold& threshold,
		uniform_real_distribution<double>& rnd_prob, uniform_int_distribution<int>& rnd_node,
		mt19937& gen, ModifyEdges& resultSet)
{
	vector<Edge>& addSet = resultSet.addSet;
	vector<Edge>& rmvSet = resultSet.rmvSet;

	int totalV = 0;
	int totalE = 0;

	string line;
	while(getline(fin, line)){
		int addCnt = 0;
		totalV++;
		// cout << line << endl;
		int u;
		vector<Edge> hs;
		tie(u, hs) = parseFromLine(line);
		maxV = max(maxV, u);
		unordered_set<int> dests;
		for(Edge& e : hs){
			dests.insert(e.v);
			maxV = max(maxV, e.v);
			double r = rnd_prob(gen);
			if(r < threshold.trivial){
				continue;
			}else if(r < threshold.add){
				++addCnt;
			}else if(r < threshold.rmv){
				rmvSet.push_back(e);
			}
		}
		totalE += hs.size();
		dests.insert(u);
		//cout << hs.size() << endl;
		// add
		while(addCnt--){
			int rpt = 0;
			int newV;
			do{
				newV = rnd_node(gen) % maxV;
			}while(dests.find(newV) != dests.end() && rpt++ < 10);
			if(rpt < 10){
				addSet.push_back(Edge{ u, newV });
			}else{
				// ++failAdd;
			}
		}
	} // line
	return make_tuple(totalV, totalE, maxV);
}

int changeGraphOnline(const string& graphFolder, const string& deltaFolder,
		const int nPart, const int seed, const double rate,
		const double addRate, const double rmvRate, const bool bidir)
{
	vector<ifstream*> fin;
	vector<ofstream*> fout;
	cout << "Loading " << nPart << " parts, from folder: " << graphFolder << endl;
	for(int i = 0; i < nPart; ++i){
		fin.push_back(new ifstream(graphFolder + "/part-" + to_string(i)));
		fout.push_back(new ofstream(deltaFolder + "/delta-" + to_string(i)));
		if(!fin.back()->is_open()){
			cerr << "failed in opening input file: " << graphFolder + "/part-" + to_string(i) << endl;
			return 0;
		}
		if(!fout.back()->is_open()){
			cerr << "failed in opening output file: " << deltaFolder + "/delta-" + to_string(i) << endl;
			return 0;
		}
	}

	chrono::time_point<std::chrono::system_clock> start_t;
	chrono::duration<double> elapsed;
	
	mt19937 gen(seed);
	uniform_real_distribution<double> rnd_prob(0.0, 1.0);
	uniform_int_distribution<int> rnd_node; // 0 to numeric_limits<int>::max()

	double addProb = rate * addRate, rmvProb = rate * rmvRate;
	ModifyThreshold threshold; //{ addTh, rmvTh, incTh, decTh };
	threshold.trivial = (1 - rate);
	threshold.add = threshold.trivial + addProb;
	threshold.rmv = threshold.add + rmvProb;

	int totalV = 0, totalE = 0;

	cout<<"Loading and generating delta information"<<endl;
    start_t = chrono::system_clock::now();
	int maxV = 0;
	ModifyEdges modifiedSet;
	for(int i = 0; i < nPart; i++){
		cout<<"  Processing "<<graphFolder + "/part-" + to_string(i)<<endl;
		// generate
		tuple<int, int, int> ret = changeOne(
				*fin[i], maxV, threshold, rnd_prob, rnd_node, gen, modifiedSet);
		totalV += get<0>(ret);
		totalE += get<1>(ret);
		maxV = max(maxV, get<2>(ret));
		delete fin[i];
	} // file
	
	if(bidir){
		cout<<"  Adding reverse edges"<<endl;
		size_t t = modifiedSet.addSet.size();
		cout<<"    RE for add: "<<addReverseEdge(modifiedSet.addSet, true)<<" / "<<t<<" \n";
		t = modifiedSet.rmvSet.size();
		cout<<"    RE for rmv: "<<addReverseEdge(modifiedSet.rmvSet, true)<<" / "<<t<<" \n";
	}
    elapsed = chrono::system_clock::now()-start_t;
    
	const int addCnt = modifiedSet.addSet.size();
	const int rmvCnt = modifiedSet.rmvSet.size();
	double te = totalE;
	cout << "Total vertex/edge: " << totalV << "/" << totalE << "\n";
	cout << "  add e: " << addCnt << "\t: " << addCnt / te << "\n";
	cout << "  rmv e: " << rmvCnt << "\t: " << rmvCnt / te << "\n";
	cout << "  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	// dump
	cout << "Dumping delta information"<<endl;
    start_t = chrono::system_clock::now();
	dumpChangeOneSet(fout, nPart, modifiedSet.addSet, 'A');
	dumpChangeOneSet(fout, nPart, modifiedSet.rmvSet, 'R');
    elapsed = chrono::system_clock::now()-start_t;
    cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	for(int i = 0; i < nPart; i++){
		delete fout[i];
	}
	return nPart;
}

// ------ offline ------

// normal return: totalE
// return by reference: resultSet = {addSet, rmvSet}
int changeAll(const vector<vector<int>>& g, const ModifyThreshold& threshold,
		uniform_real_distribution<double>& rnd_prob, uniform_int_distribution<int>& rnd_node,
		mt19937& gen, ModifyEdges& resultSet)
{
	vector<Edge>& addSet = resultSet.addSet;
	vector<Edge>& rmvSet = resultSet.rmvSet;

	int totalV = g.size();
	int totalE = 0;
	
	for(size_t i = 0; i < g.size(); ++i){
		int addCnt = 0;
		const auto& vec = g[i];
		for(const int& j : vec){
			double r = rnd_prob(gen);
			if(r < threshold.trivial){
				continue;
			}else if(r < threshold.add){
				++addCnt;
			}else if(r < threshold.rmv){
				rmvSet.push_back(Edge{i, j});
			}
		}
		totalE += vec.size();
		// add
		while(addCnt--){
			int rpt = 0;
			int newV;
			do{
				newV = rnd_node(gen);
			}while(binary_search(vec.begin(), vec.end(), newV) && rpt++ < 10);
			if(rpt < 10){
				Edge e{ i, newV};
				addSet.push_back(e);
			}else{
				// ++failAdd;
			}
		}
	}
	return totalE;
}

int changeGraphOffline(const string& graphFolder, const string& deltaFolder,
		const int nPart, const int seed, const double rate,
		const double addRate, const double rmvRate, const bool bidir)
{
	vector<ofstream*> fout;
	for(int i = 0; i < nPart; ++i){
		fout.push_back(new ofstream(deltaFolder + "/delta-" + to_string(i)));
		if(!fout.back()->is_open()){
			cerr << "failed in opening output file: " << deltaFolder + "/delta-" + to_string(i) << endl;
			return 0;
		}
	}
	
	chrono::time_point<std::chrono::system_clock> start_t;
	chrono::duration<double> elapsed;
	// load
	cout << "Loading " << nPart << " parts, from folder: " << graphFolder << endl;
    start_t = chrono::system_clock::now();
	vector<vector<int>> g;
	try{
		g = general_load_unweight(nPart, graphFolder, "part-");
	}catch(exception& e){
		cerr<<e.what()<<endl;
		return 3;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  load "<<g.size()<<" nodes in "<<elapsed.count()<<" seconds"<<endl;
	
	// generate delta information
	mt19937 gen(seed);
	uniform_real_distribution<double> rnd_prob(0.0, 1.0);
	uniform_int_distribution<int> rnd_node(0, g.size());

	double addProb = rate * addRate, rmvProb = rate * rmvRate;

	ModifyThreshold threshold; //{ addTh, rmvTh };
	threshold.trivial = (1 - rate);
	threshold.add = threshold.trivial + addProb;
	threshold.rmv = threshold.add + rmvProb;

	
	cout<<"Generating delta information"<<endl;
    start_t = chrono::system_clock::now();
	ModifyEdges modifiedSet;
	int totalE = changeAll(g, threshold, rnd_prob, rnd_node, gen, modifiedSet);
	if(bidir){
		cout<<"  Adding reverse edges"<<endl;
		size_t t = modifiedSet.addSet.size();
		cout<<"    RE for add: "<<addReverseEdge(modifiedSet.addSet, true)<<" / "<<t<<" \n";
		t = modifiedSet.rmvSet.size();
		cout<<"    RE for rmv: "<<addReverseEdge(modifiedSet.rmvSet, true)<<" / "<<t<<" \n";
	}
    elapsed = chrono::system_clock::now()-start_t;
    
	const int addCnt = modifiedSet.addSet.size();
	const int rmvCnt = modifiedSet.rmvSet.size();
	const int totalV = g.size();
	const double te = totalE;
	cout << "Total vertex/edge: " << totalV << "/" << totalE << "\n";
	cout << "  add e: " << addCnt << "\t: " << addCnt / te << "\n";
	cout << "  rmv e: " << rmvCnt << "\t: " << rmvCnt / te << "\n";
	cout << "  finished in "<<elapsed.count()<<" seconds"<<endl;

	// dump delta information
	cout << "Dumping delta information"<<endl;
    start_t = chrono::system_clock::now();
	dumpChangeOneSet(fout, nPart, modifiedSet.addSet, 'A');
	dumpChangeOneSet(fout, nPart, modifiedSet.rmvSet, 'R');
    elapsed = chrono::system_clock::now()-start_t;
    cout<<"  finished in "<<elapsed.count()<<" seconds"<<endl;
	
	for(int i = 0; i < nPart; i++){
		delete fout[i];
	}

	return nPart;
}

// ------ main ------

struct Option{
	string graphFolder;
	int nPart;
	string deltaFolder;
	
	double alpha; // for power-law distribution
	
	string weight;
	double wmin, wmax;
	double rate;	// rate of changed edges
	double addRate, rmvRate;

	bool online;
	bool dir;
	unsigned long seed;
	
	void parse(int argc, char* argv[]);
private:
	bool setWeight(string& method);
	bool checkRate1(double rate);
	bool checkRate2(double rate);
	bool normalizeRates();
};

void Option::parse(int argc, char* argv[]){
	nPart = stoi(string(argv[1]));
	graphFolder = argv[2];
//	nNode=stoi(string(argv[2]));
	deltaFolder = argv[3];
	rate = stod(string(argv[4]));
	addRate = stod(string(argv[5]));
	rmvRate = stod(string(argv[6]));
	online = false;
	if(argc > 7)
		online = beTrueOption(string(argv[7]));
	dir = true;
	if(argc > 8)
		dir = beTrueOption(string(argv[8]));
	seed = 1535345;
	if(argc > 9)
		seed = stoul(string(argv[9]));
	if(!normalizeRates())
		throw invalid_argument("Given rates do not make sense.");
}
bool Option::checkRate1(double rate){
	return 0.0 <= rate;
}
bool Option::checkRate2(double rate){
	return 0.0 <= rate && rate <= 1.0;
}
bool Option::normalizeRates(){
	bool flag = checkRate2(rate)
			&& checkRate1(addRate) && checkRate1(rmvRate);
	if(!flag)
		return false;
	double total = addRate + rmvRate;
	if(total != 1.0){
		cout << "normalizing modifying rates" << endl;
		addRate /= total;
		rmvRate /= total;
	}
	return true;
}

int main(int argc, char* argv[]){
	if(argc < 7 || argc > 10){
		cerr << "Generate delta information for unweighted graph.\n"
				"Usage: <#parts> <graph-folder> <delta-folder> <deltaRate> <addRate> <rmvRate> [online] [dir] [random-seed]"
				<< endl;
		cerr <<	"  <#parts>: number of parts the graphs are separated (the number of files to operate).\n"
				"  <graph-folder>: the folder of graphs, naming format: \"<graph-folder>/part-<part>\".\n"
				"  <delta-folder>: the folder of generated delta information, naming format: \"<delta-folder>/delta-<part>\".\n"
				"  <deltaRate>: the rate of changed edges.\n"
				"  <addRate>, <rmvRate>: "
				"among the changed edges the rates for edge-addition and edge-removal. "
				"They are automatically normalized.\n"
				"  [online]: (=0) whether to perform online generation. "
				"Offline version guarantees equivalent output for same graph and seed."
				"Online version guarantees that ONLY when <#part> is also identical (optimized for huge graph).\n"
				"  [dir]: (=1) whether it is a directional graph\n"
				"  [random-seed]: (=1535345) seed for random numbers\n"
				"i.e.: ./delta-gen-uw.exe 1 graphDir delta-rd 0.05 0 1 0 1 123456\n"
				"i.e.: ./delta-gen-uw.exe 2 input ../delta/ 0.01 0.2 0.8 1 0\n"
				<< endl;
		return 1;
	}
	Option opt;
	try{
		opt.parse(argc, argv);
	} catch(exception& e){
		cerr << e.what() << endl;
		return 2;
	}
	ios_base::sync_with_stdio(false);

	int n = 0;
	if(opt.online){
		n = changeGraphOnline(opt.graphFolder, opt.deltaFolder, opt.nPart, opt.seed, opt.rate,
			opt.addRate, opt.rmvRate, !opt.dir);
	}else{
		n = changeGraphOffline(opt.graphFolder, opt.deltaFolder, opt.nPart, opt.seed, opt.rate,
			opt.addRate, opt.rmvRate, !opt.dir);
	}

	cout << "success " << n << " files. fail " << opt.nPart - n << " files." << endl;
	return n > 0 ? 0 : 3;
}
