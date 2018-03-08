#include "GraphContainer.h"
#include "util/FileEnumerator.h"
#include <vector>
#include <regex>
#include <fstream>

using namespace std;

GraphContainer::GraphContainer(AppBase& app, const ConfData& conf)
	: app(app), conf(conf), holder(nullptr)
{
}
GraphContainer::~GraphContainer(){
	delete holder;
}
void GraphContainer::init(int wid, GlobalHolderBase* holder){
	this->wid = wid;
	this->holder = holder;
}

void GraphContainer::loadGraph(){
	vector<string> files = FileEnumerator::listFile(conf.path_graph, conf.prefix_graph);
	if(!conf.balance_load && files.size() != conf.nPart){
		throw invalid_argument("Graph files do not match workers. Consider turning on <balance_load>.");
	}
	regex reg(conf.prefix_graph+"(\\d+)");
	for(auto& fn : files){
		smatch m;
		if(regex_match(fn, m, reg)){
			int id = stoi(m[1].str());
			if(id == wid || (conf.balance_load && id%conf.nPart == wid))
				loadGraphFile(conf.path_graph + "/" + fn);
		}
	}
}

void GraphContainer::loadValue(){
	vector<string> files = FileEnumerator::listFile(conf.path_value, conf.prefix_value);
	if(!conf.balance_load && files.size() != conf.nPart){
		throw invalid_argument("Graph files do not match workers. Consider turning on <balance_load>.");
	}
	regex reg(conf.prefix_value+"(\\d+)");
	for(auto& fn : files){
		smatch m;
		if(regex_match(fn, m, reg)){
			int id = stoi(m[1].str());
			if(id == wid || (conf.balance_load && id%conf.nPart == wid))
				loadValueFile(conf.path_value + "/" + fn);
		}
	}
}

void GraphContainer::loadDelta(){
	vector<string> files = FileEnumerator::listFile(conf.path_delta, conf.prefix_delta);
	if(!conf.balance_load && files.size() != conf.nPart){
		throw invalid_argument("Graph files do not match workers. Consider turning on <balance_load>.");
	}
	regex reg(conf.prefix_delta+"(\\d+)");
	for(auto& fn : files){
		smatch m;
		if(regex_match(fn, m, reg)){
			int id = stoi(m[1].str());
			if(id == wid || (conf.balance_load && id%conf.nPart == wid))
				loadDeltaFile(conf.path_delta + "/" + fn);
		}
	}
}

void GraphContainer::update(){

}

void GraphContainer::output(){
	string fn = conf.prefix_result + to_string(wid);
	holder->output(conf.path_result + "/" + fn);
}

// --------

void GraphContainer::loadGraphFile(const std::string& fn){
	ifstream fin(fn);
	string line;
	while(getline(fin, line)){
		holder->loadGraph(line);
	}
}
void GraphContainer::loadValueFile(const std::string& fn){
	ifstream fin(fn);
	string line;
	while(getline(fin, line)){
		holder->loadValue(line);
	}
}
void GraphContainer::loadDeltaFile(const std::string& fn){
	ifstream fin(fn);
	string line;
	while(getline(fin, line)){
		holder->loadDelta(line);
	}
}
void GraphContainer::loadGraphPiece(const std::string& line){
	holder->loadGraph(line);
}
void GraphContainer::loadValuePiece(const std::string& line){
	holder->loadValue(line);
}
void GraphContainer::loadDeltaPiece(const std::string& line){
	holder->loadDelta(line);
}

