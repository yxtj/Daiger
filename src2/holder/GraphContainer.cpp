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

void GraphContainer::init(int wid, GlobalHolderBase* holder, bool incremental){
	this->wid = wid;
	this->holder = holder;
	holder->init(app.opt, app.ioh, app.scd, app.ptn, app.tmt,
		conf.nPart, wid, conf.aggregate_message,
		incremental, conf.async, conf.cache_free, conf.sort_result,
		conf.send_min_size, conf.send_max_size);
	applying = false;
	sending = false;
}

void GraphContainer::loadGraph(sender_t sender){
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
				loadGraphFile(conf.path_graph + "/" + fn, sender);
		}
	}
	holder->addDummyNodes();
}

void GraphContainer::loadValue(sender_t sender){
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
				loadValueFile(conf.path_value + "/" + fn, sender);
		}
	}
}

void GraphContainer::loadDelta(sender_t sender){
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
				loadDeltaFile(conf.path_delta + "/" + fn, sender);
		}
	}
}

void GraphContainer::dumpResult(){
	if(!FileEnumerator::ensureDirectory(conf.path_result)){
		throw invalid_argument("Cannot access or create output folder \"" + conf.path_result + "\"");
	}
	string fn = conf.path_result + conf.prefix_result + to_string(wid);
	ofstream fout(fn);
	if(!fout){
		throw runtime_error("Cannot create output file \"" + fn + "\"");
	}
	holder->prepareDump();
	std::pair<bool, std::string> p = holder->dumpResult();
	while(p.first){
		fout<<p.second<<"\n";
		p = holder->dumpResult();	
	}
}

void GraphContainer::buildINCache(sender_t sender){
	holder->prepareCollectINCache();
	unordered_map<int, string> tmp = holder->collectINCache();
	for(auto& p : tmp){
		if(!p.second.empty()){
			sender(p.first, p.second);
		}
	}
}

void GraphContainer::rebuildSource(){
	holder->rebuildSource();
}

void GraphContainer::clearINCache(){
	holder->clearINCache();
}

void GraphContainer::genInitMsg(){
	holder->intializedProcess();
}

void GraphContainer::prepareUpdate(sender_t sender_val, sender_t sender_req, sender0_t sender_pro){
	this->sender_val = sender_val;
	this->sender_req = sender_req;
	this->sender_pro = sender_pro;
	holder->prepareUpdate(sender_req);
}

void GraphContainer::apply(){
	if(!applying && holder->needApply()){
		applying = true;
		holder->doApply();
		applying = false;
	}
}
void GraphContainer::send(){
	if(sending || !holder->needSend(tmr_send.elapseSd() > conf.send_max_interval))
		return;
	tmr_send.restart();
	sending = true;
	for(int i=0; i<conf.nPart; ++i){
		if(i == wid)
			continue;
		string msg = holder->collectMsg(i);
		sender_val(i, msg);
	}
	sending = false;
}

void GraphContainer::reportProgress(){
	string progress = holder->collectLocalProgress();
	sender_pro(progress);
}
// --------

void GraphContainer::loadGraphFile(const std::string& fn, sender_t sender){
	ifstream fin(fn);
	string line;
	while(getline(fin, line)){
		int pid = holder->loadGraph(line);
		if(pid != wid)
			sender(pid, line);
	}
}
void GraphContainer::loadValueFile(const std::string& fn, sender_t sender){
	ifstream fin(fn);
	string line;
	while(getline(fin, line)){
		int pid = holder->loadValue(line);
		if(pid != wid)
			sender(pid, line);
	}
}
void GraphContainer::loadDeltaFile(const std::string& fn, sender_t sender){
	ifstream fin(fn);
	string line;
	while(getline(fin, line)){
		if(line.size() < 3)
			continue;
		int pid = holder->loadDelta(line);
		if(pid != wid)
			sender(pid, line);
	}
}
bool GraphContainer::loadGraphPiece(const std::string& line){
	return holder->loadGraph(line) == wid;
}
bool GraphContainer::loadValuePiece(const std::string& line){
	return holder->loadValue(line) == wid;
}
bool GraphContainer::loadDeltaPiece(const std::string& line){
	return holder->loadDelta(line) == wid;
}

void GraphContainer::takeINCache(const std::string& line){
	holder->takeINCache(line);
}

void GraphContainer::msgUpdate(const std::string& line){
	holder->msgUpdate(line);
}
void GraphContainer::msgRequest(const std::string& line){
	holder->msgRequest(line);
}
void GraphContainer::msgReply(const std::string& line){
	holder->msgReply(line);
}
