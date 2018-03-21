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
	setLogThreadName("W"+to_string(my_net_id));
}

void Worker::run() {
	registerHandlers();
	startMsgLoop("W"+to_string(my_net_id)+"-MSG");
    registerWorker();

	su_stop.wait(); // wait for handleShutdown which calls shutdownWorker

	// finish
	stopMsgLoop();
    tmsg.join();
    shutdownWorker();
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

void Worker::clearMessages(){
	net->flush();
	while(!driver.empty()){
		sleep();
		sleep();
	}
	net->flush();
	DLOG(DEBUG)<<"messages get cleared";
}

void Worker::storeWorkerInfo(const std::vector<std::pair<int, int>>& winfo){
	for(auto& p : winfo){
		wm.register_worker(p.first, p.second);
	}
	LOG(INFO)<<"worker information received";
	su_winfo.notify();
}

void Worker::procedureInit(){
	// notified by handleWorkers()
	su_winfo.wait();
	graph.init(wm.nid2wid(my_net_id), app.gh);
}

void Worker::procedureLoadGraph(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GNode, move(msg));
		};
	VLOG(1)<<"worker start loading graph";
	graph.loadGraph(sender);
}

void Worker::procedureLoadValue(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GValue, move(msg));
		};
	VLOG(1)<<"worker start loading value";
	graph.loadValue(sender);
}

void Worker::procedureLoadDelta(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GDelta, move(msg));
		};
	VLOG(1)<<"worker start loading delta";
	graph.loadDelta(sender);
}

void Worker::procedureBuildINCache(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GINCache, move(msg));
		};
	VLOG(1)<<"worker start building in-neighbor cache";
	graph.buildINCache(sender);
}

void Worker::reportProgress(){
	std::function<void(std::string&)> sender = 
		[&](std::string& msg){
			net->send(master_net_id, MType::PReport, move(msg));
		};
	graph.reportProgress(sender);
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
	VLOG(1)<<"worker start updating";
	graph.prepareUpdate(sender_val, sender_req);
	// start periodic apply-and-send and periodic progress-report
	update_finish=false;
	int ams = static_cast<int>(opt.apply_interval*1000); // millisecond
	int tms = static_cast<int>(opt.term_interval*1000);
	double interval = _helper_gcd(ams, tms) / 1000.0;
	Timer last_apply, last_term;
	RPCInfo info;
	info.source = net->id();
	info.dest = net->id();
	while(!update_finish){
		if(!su_update.wait_for(interval)){ // wake up by timeout
			if(last_apply.elapseMS() > ams){
				last_apply.restart();
				info.tag = MType::PApply;
				driver.pushData("", info);
			}
			//TODO: separate apply and send
			if(last_term.elapseMS() > tms){
				last_term.restart();
				info.tag = MType::PReport;
				driver.pushData("", info);
			}
		}else{ // wake up by termination singal
			break;
		}
	}
}

void Worker::procedureDumpResult(){
	VLOG(1)<<"worker start dumping result";
	graph.dumpResult();
}

