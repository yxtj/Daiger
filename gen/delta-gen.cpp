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
#include <chrono>
#include "common.h"

using namespace std;

// ---- load the graph data and generate the delta file

struct Edge{
	int u, v;
	float w;
};

struct ModifyThreshold{
	double trivial;
	double add;
	double rmv;
	double inc;
	double dec;
};

struct ModifyEdges{
	vector<Edge> addSet;
	vector<Edge> rmvSet;
	vector<Edge> incSet;
	vector<Edge> decSet;
};

// normal return: totalE
// return by reference: resultSet = {addSet, rmvSet, incSet, decSet}
int changeOne(const vector<vector<Link>>& g, const ModifyThreshold& threshold,
		uniform_real_distribution<double>& rnd_prob, uniform_int_distribution<int>& rnd_node,
		uniform_real_distribution<float>& rnd_weight, mt19937& gen,
		ModifyEdges& resultSet)
{
	vector<Edge>& addSet = resultSet.addSet;
	vector<Edge>& rmvSet = resultSet.rmvSet;
	vector<Edge>& incSet = resultSet.incSet;
	vector<Edge>& decSet = resultSet.decSet;

	int totalV = g.size();
	int totalE = 0;
	const int maxV = g.size() - 1;
	
	for(size_t i = 0; i < g.size(); ++i){
		int addCnt = 0;
		const auto& vec = g[i];
		for(const Link& e : vec){
			double r = rnd_prob(gen);
			if(r < threshold.trivial){
				continue;
			}else if(r < threshold.add){
				++addCnt;
			}else if(r < threshold.rmv){
				rmvSet.push_back(Edge{i, e.node, e.weight});
			}else if(r < threshold.inc){
				float w = e.weight * (1 + rnd_weight(gen));
				rmvSet.push_back(Edge{i, e.node, w});
			}else if(r < threshold.dec){
				float w = e.weight * rnd_weight(gen);
				rmvSet.push_back(Edge{i, e.node, w});
			}
		}
		totalE += vec.size();
		// add
		while(addCnt--){
			int rpt = 0;
			int newV;
			do{
				newV = rnd_node(gen) % maxV;
			}while(find_if(vec.begin(), vec.end(), [&](const Link& e){ return e.node == newV; }) != vec.end() && rpt++ < 10);
			if(rpt < 10){
				Edge e{ i, newV, rnd_weight(gen) };
				addSet.push_back(e);
			}else{
				// ++failAdd;
			}
		}
	}
	return totalE;
}

void addReverseEdge(vector<Edge>& es){
	vector<pair<int, int>> tmp;
	tmp.reserve(es.size());
	for(Edge& e : es){
		if(e.u < e.v)
			tmp.emplace_back(e.u, e.v);
		else
			tmp.emplace_back(e.v, e.u);
	}
	sort(tmp.begin(), tmp.end());
	vector<pair<int, int>> ext; // those already have a reverse edge in es
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
			es.push_back(Edge{e.v, e.u, e.w});
		}
	}else{
		for(size_t i = 0; i<n; ++i){
			const auto& e = es[i];
			pair<int, int> t = e.u < e.v ? make_pair(e.u, e.v) : make_pair(e.v, e.u);
			if(find(tmp.begin(), tmp.end(), t) == tmp.end())
				es.push_back(Edge{e.v, e.u, e.w});
		}
	}
	sort(es.begin(), es.end(), [](const Edge& l, const Edge& r){
		return l.u < r.u ? true : l.u == r.u && l.v < r.v;
	});
}

void dumpChangeOneSet(vector<ofstream*>& fouts, const int n, const vector<Edge>& es, char type){
	for(const Edge& e : es){
		(*fouts[e.u % n]) << type << " " << e.u << "," << e.v << "," << e.w << "\n";
	}
}

int changeGraph(const string& graphFolder, const string& deltaFolder,
		const int nPart, const int seed, const double rate,
		const double addRate, const double rmvRate, const double incRate, const double decRate, const bool bidir)
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
	vector<vector<Link>> g;
	try{
		g = general_load_weight(nPart, graphFolder, "part-");
	}catch(exception& e){
		cerr<<e.what()<<endl;
		return 3;
	}
    elapsed = chrono::system_clock::now()-start_t;
	cout<<"  load "<<g.size()<<" nodes in "<<elapsed.count()<<" seconds"<<endl;
	
	// generate delta information
	mt19937 gen(seed);
	uniform_real_distribution<double> rnd_prob(0.0, 1.0);
	uniform_int_distribution<int> rnd_node; // 0 to numeric_limits<int>::max()
	uniform_real_distribution<float> rnd_weight(0, 1);

	//double modProb=rate*(1-addRate);
	double addProb = rate * addRate, rmvProb = rate * rmvRate;
	double incProb = rate * incRate, decProb = rate * decRate;

	ModifyThreshold threshold; //{ addTh, rmvTh, incTh, decTh };
	threshold.trivial = (1 - rate);
	threshold.add = threshold.trivial + addProb;
	threshold.rmv = threshold.add + rmvProb;
	threshold.inc = threshold.rmv + incProb;
	threshold.dec = threshold.inc + decProb;

	cout<<"Generating delta information"<<endl;
    start_t = chrono::system_clock::now();
	ModifyEdges modifiedSet;
	int totalE = changeOne(g, threshold, rnd_prob, rnd_node, rnd_weight, gen, modifiedSet);
	if(bidir){
		addReverseEdge(modifiedSet.addSet);
		addReverseEdge(modifiedSet.rmvSet);
		addReverseEdge(modifiedSet.incSet);
		addReverseEdge(modifiedSet.decSet);
	}
    elapsed = chrono::system_clock::now()-start_t;
    
	const int addCnt = modifiedSet.addSet.size();
	const int rmvCnt = modifiedSet.rmvSet.size();
	const int incCnt = modifiedSet.incSet.size();
	const int decCnt = modifiedSet.decSet.size();
	int totalV = g.size();
	double te = totalE;
	cout << "Total vertex/edge: " << totalV << "/" << totalE << "\n";
	cout << "  add e: " << addCnt << "\t: " << addCnt / te << "\n";
	cout << "  rmv e: " << rmvCnt << "\t: " << rmvCnt / te << "\n";
	cout << "  inc w: " << incCnt << "\t: " << incCnt / te << "\n";
	cout << "  dec w: " << decCnt << "\t: " << decCnt / te << "\n";
	cout << "  finished in "<<elapsed.count()<<" seconds"<<endl;

	// dump delta information
	cout << "Dumping delta information"<<endl;
    start_t = chrono::system_clock::now();
	dumpChangeOneSet(fout, nPart, modifiedSet.addSet, 'A');
	dumpChangeOneSet(fout, nPart, modifiedSet.rmvSet, 'R');
	dumpChangeOneSet(fout, nPart, modifiedSet.incSet, 'I');
	dumpChangeOneSet(fout, nPart, modifiedSet.decSet, 'D');
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
	double addRate, rmvRate, incRate, decRate;

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
	incRate = stod(string(argv[7]));
	decRate = stod(string(argv[8]));
	dir = true;
	if(argc > 9)
		dir = beTrueOption(string(argv[9]));
	seed = 1535345;
	if(argc > 10)
		seed = stoul(string(argv[10]));
	if(!normalizeRates())
		throw invalid_argument("Given rates do not make sense.");
}
bool Option::setWeight(string& method){
	if(method == "no"){
		weight = "no";
	}else if(method.substr(0, 7) == "weight:"){
		weight = "weight";
		size_t p = method.find(',', 7);
		wmin = stod(method.substr(7, p - 7));
		wmax = stod(method.substr(p + 1));
	}else{
		return false;
	}
	return true;
}
bool Option::checkRate1(double rate){
	return 0.0 <= rate;
}
bool Option::checkRate2(double rate){
	return 0.0 <= rate && rate <= 1.0;
}
bool Option::normalizeRates(){
	bool flag = checkRate2(rate)
			&& checkRate1(addRate) && checkRate1(rmvRate)
			&& checkRate1(incRate) && checkRate1(decRate);
	if(!flag)
		return false;
	double total = addRate + rmvRate + incRate + decRate;
	if(total != 1.0){
		cout << "normalizing modifying rates" << endl;
		addRate /= total;
		rmvRate /= total;
		incRate /= total;
		decRate /= total;
	}
	return true;
}

int main(int argc, char* argv[]){
	if(argc < 9 || argc > 11){
		cerr << "Generate delta information. Result with same input graph and seed are guaranteed to be identical\n"
				"Usage: <#parts> <graph-folder> <delta-folder> <deltaRate> <addRate> <rmvRate> <incRate> <decRate> [dir] [random-seed]"
				<< endl;
		cerr <<	"  <#parts>: number of parts the graphs are separated (the number of files to operate).\n"
				"  <graph-folder>: the folder of graphs, naming format: \"<graph-folder>/part-<part>\".\n"
				"  <delta-folder>: the folder of generated delta information, naming format: \"<delta-folder>/delta-<part>\".\n"
				"  <deltaRate>: the rate of changed edges.\n"
				"  <addRate>, <rmvRate>, <incRate>, <decRate>: "
				"among the changed edges the rates for edge-addition, edge-removal, weight-increase and weight-decrease. "
				"They are automatically normalized.\n"
				"  [dir]: (=1) whether it is a directional graph\n"
				"  [random-seed]: (=1535345) seed for random numbers\n"
				"i.e.: ./delta-gen.exe 1 graphDir delta-rd 0.05 0 0.3 0 0.7 1 123456\n"
				"i.e.: ./delta-gen.exe 2 input ../delta/d2 0.01 0.2 0.2 0.3 0.3\n"
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

	int n = changeGraph(opt.graphFolder, opt.deltaFolder, opt.nPart, opt.seed, opt.rate,
			opt.addRate, opt.rmvRate, opt.incRate, opt.decRate, !opt.dir);

	cout << "success " << n << " files. fail " << opt.nPart - n << " files." << endl;
	return n > 0 ? 0 : 3;
}

