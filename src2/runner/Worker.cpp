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
	VLOG(1)<<"Worker start loading graph";
	graph.loadGraph(sender);
}

void Worker::procedureLoadValue(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GValue, move(msg));
		};
	setLogThreadName(log_name+"-PLV");
	VLOG(1)<<"Worker start loading value";
	graph.loadValue(sender);
}

void Worker::procedureLoadDelta(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GDelta, move(msg));
		};
	setLogThreadName(log_name+"-PLD");
	VLOG(1)<<"Worker start loading delta";
	graph.loadDelta(sender);
}

void Worker::procedureBuildINCache(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GINCache, move(msg));
		};
	setLogThreadName(log_name+"-BIC");
	VLOG(1)<<"Worker start building in-neighbor cache";
	graph.buildINCache(sender);
}

void Worker::procedureRebuildStructure(){
	setLogThreadName(log_name+"-RSS");
	VLOG(1)<<"Worker start reconstructing source structure";
	graph.rebuildSource();
	if(opt.conf.cache_free)
		graph.clearINCache();
}

void Worker::procedureGenInitMsg(){
	setLogThreadName(log_name+"-GIM");
	VLOG(1)<<"Worker start generating initial messages";
	graph.genInitMsg();
}

void Worker::reportProgress(){
	VLOG(2)<<"sending progress report";
	graph.reportProgress();
}

static int _helper_gcd(int a, int b){
	return b == 0 ? a : _helper_gcd(b, a % b);
}
void Worker::procedureUpdate(){
	// register useful callbacks for sending messages
	std::function<void(const int, std::string&)> sender_val = 
		[&](const int wid, std::string& msg){
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

	// start periodic apply-and-send and periodic progress-report
	update_finish=false;
	int ams = static_cast<int>(opt.apply_interval*1000); // millisecond
	int sms = static_cast<int>(opt.send_interval*1000);
	int tms = static_cast<int>(opt.term_interval*1000);
	double interval = _helper_gcd(_helper_gcd(ams, sms), tms) / 1000.0;
	Timer last_apply, last_send, last_term;
	RPCInfo info;
	info.source = my_net_id;
	info.dest = my_net_id;
	// start with an apply and send
	info.tag = MType::PApply;
	driver.pushData("", info);
	info.tag = MType::PSend;
	driver.pushData("", info);
	while(!update_finish){
		if(!su_update.wait_for(interval)){ // wake up by timeout
			su_update.reset();
			if(!update_finish && last_apply.elapseMS() > ams){
				last_apply.restart();
				info.tag = MType::PApply;
				driver.pushData("", info);
			}
			if(!update_finish && last_send.elapseMS() > sms){
				last_send.restart();
				info.tag = MType::PSend;
				driver.pushData("", info);
			}
			if(!update_finish && last_term.elapseMS() > tms){
				last_term.restart();
				info.tag = MType::PReport;
				driver.pushData("", info);
			}
		}else{ // wake up by termination singal
			LOG(INFO)<<"Force finish updating by master";
			break;
		}
	}
}

void Worker::procedureDumpResult(){
	setLogThreadName(log_name+"-PDR");
	VLOG(1)<<"Worker start dumping result";
	graph.dumpResult();
}

