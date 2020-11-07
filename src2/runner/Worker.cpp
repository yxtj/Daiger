#include "Worker.h"
#include "msg/MType.h"
#include "network/NetworkThread.h"
#include "util/Timer.h"
#include "logging/logging.h"
#include <functional>
#include <chrono>

using namespace std;

Worker::Worker(AppBase& app, Option& opt)
	: Runner(app, opt), graph(app, opt.conf)
{
	my_net_id = net->id();
	log_name = "W"+to_string(my_net_id);
	setLogThreadName(log_name);
}

void Worker::run() {
	registerHandlers();
	startMsgLoop(log_name+"-MSG");
    registerWorker();

	su_stop.wait(); // wait for handleShutdown which calls shutdownWorker
	if(tprcd.joinable())
		tprcd.join();
	clearMessages();
	stopMsgLoop();
}

void Worker::registerWorker(){
	// wait for the master sending CRegister
	su_master.wait();
	// called by handleRegister()
	LOG(INFO)<<"registering worker with net id: "<<my_net_id;
	net->send(master_net_id, MType::CRegister, my_net_id);
	su_regw.wait(); // notified by the reply of CRegister
	LOG(INFO)<<"worker registered.";
}

void Worker::shutdownWorker(){
	LOG(INFO)<<"Worker "<<wm.nid2wid(my_net_id)<<" stops.";
	su_stop.notify();
}

void Worker::terminateWorker(){
	LOG(ERROR)<<"Terminated by Master.";
	NetworkThread::Terminate();
	exit(0);
}

void Worker::storeWorkerInfo(const std::vector<std::pair<int, int>>& winfo){
	for(auto& p : winfo){
		wm.register_worker(p.first, p.second);
	}
	LOG(INFO)<<"Worker information received";
	su_winfo.notify();
}

void Worker::procedureInit(){
	setLogThreadName(log_name+"-INT");
	// notified by handleWorkers() via storeWorkerInfo()
	su_winfo.wait();
	graph.init(wm.nid2wid(my_net_id), app.gh, opt.do_incremental);
	LOG(INFO)<<"Graph initialized";
}

void Worker::procedureLoadGraph(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GNode, move(msg));
		};
	setLogThreadName(log_name+"-PLG");
	LOG(INFO) << "Worker start loading graph";
	pair<int, int> nlr = graph.loadGraph(sender);
	// report
	net->send(master_net_id, MType::HGraphSize, nlr);
	// load balance
	if(opt.conf.balance_load && opt.conf.nPart > 1){
		for(int i = 0; i < opt.conf.nPart; ++i){
			net->send(wm.wid2nid(i), MType::HLoadBalance, "graph");
		}
		su_loadbalance.wait_reset();
		VLOG(2) << "load balance for graph on W" << wm.nid2wid(my_net_id) << " finished";
	}
	graph.finishGraph();
}


void Worker::procedureLoadValue(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GValue, move(msg));
		};
	setLogThreadName(log_name+"-PLV");
	LOG(INFO) << "Worker start loading value";
	graph.loadValue(sender);
	// load balance
	if(opt.conf.balance_load && opt.conf.nPart > 1){
		for(int i = 0; i < opt.conf.nPart; ++i){
			if(i == wm.nid2wid(my_net_id))
				rph.input(MType::HLoadBalance, i);
			net->send(wm.wid2nid(i), MType::HLoadBalance, "value");
		}
		su_loadbalance.wait_reset();
	}
}

void Worker::procedureLoadDelta(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GDelta, move(msg));
		};
	setLogThreadName(log_name+"-PLD");
	LOG(INFO) << "Worker start loading delta";
	graph.loadDelta(sender);
	// load balance
	if(opt.conf.balance_load && opt.conf.nPart > 1){
		for(int i = 0; i < opt.conf.nPart; ++i){
			if(i == wm.nid2wid(my_net_id))
				rph.input(MType::HLoadBalance, i);
			net->send(wm.wid2nid(i), MType::HLoadBalance, "delta");
		}
		su_loadbalance.wait_reset();
	}
}

void Worker::procedureBuildINList()
{
	std::function<void(const int, std::string&)> sender =
		[&](const int wid, std::string& msg){
		net->send(wm.wid2nid(wid), MType::GINList, move(msg));
	};
	setLogThreadName(log_name + "-BIL");
	LOG(INFO) << "Worker start building in-neighbor list";
	graph.buildINList(sender);
}

void Worker::procedureBuildINCache(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GINCache, move(msg));
		};
	setLogThreadName(log_name+"-BIC");
	LOG(INFO) << "Worker start building in-neighbor cache";
	graph.buildINCache(sender);
}

void Worker::procedureRebuildStructure(){
	setLogThreadName(log_name+"-RSS");
	LOG(INFO) << "Worker start reconstructing source structure";
	graph.rebuildSource();
	if(opt.conf.cache_free)
		graph.clearINCache();
}

void Worker::procedureGenInitMsg(){
	setLogThreadName(log_name+"-GIM");
	LOG(INFO) << "Worker start generating initial messages";
	graph.genInitMsg();
}

static int _helper_gcd(int a, int b){
	return b == 0 ? a : _helper_gcd(b, a % b);
}
void Worker::procedureUpdate(){
	// register useful callbacks for sending messages
	std::function<void(const int, std::string&)> sender_val = 
		[&](const int wid, std::string& msg){
			//VLOG(2) << "send to " << wid;
			net->send(wm.wid2nid(wid), MType::VUpdate, move(msg));
		};
	std::function<void(const int, std::string&)> sender_req = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::VRequest, move(msg));
		};
	std::function<void(std::string&)> sender_pro = 
		[&](std::string& msg){
			net->send(master_net_id, MType::PReport, move(msg));
		};
	setLogThreadName(log_name+"-PU");
	VLOG(1)<<"Worker start updating";
	graph.prepareUpdate(sender_val, sender_req, sender_pro);

	if(opt.conf.async){
		graph.updateAsync();
	} else{
		graph.updateSync();
	}
}

void Worker::procedureDumpResult(){
	setLogThreadName(log_name+"-PDR");
	LOG(INFO) << "Worker start dumping result";
	graph.dumpResult();
}

