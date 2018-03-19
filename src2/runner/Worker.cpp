#include "Worker.h"
#include "msg/MType.h"
#include "network/NetworkThread.h"
#include "util/Timer.h"
#include <functional>
#include <chrono>
#include <iostream>

using namespace std;

Worker::Worker(AppBase& app, Option& opt)
	: Runner(app, opt), graph(app, opt.conf)
{
	my_net_id = net->id();
}

void Worker::start() {
	registerHandlers();
	startMsgLoop();
    registerWorker();
	su_stop.wait();
}

void Worker::finish() {
	stopMsgLoop();
    tmsg.join();
    shutdownWorker();
}

void Worker::registerWorker(){
	// called by handleRegister()
	cout<<"registing worker with net id: "<<my_net_id<<endl;
	net->send(master_net_id, MType::CRegister, net->id());
}

void Worker::shutdownWorker(){
	cout<<"Worker "<<wm.nid2wid(my_net_id)<<" stops."<<endl;
	su_stop.notify();
}

void Worker::terminateWorker(){
	cerr<<"Terminated by Master."<<endl;
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
}

void Worker::procedureInit(){
	// notified by handleWorkers()
	su_worker.wait();
	graph.init(wm.nid2wid(my_net_id), app.gh);
}

void Worker::procedureLoadGraph(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GNode, move(msg));
		};
	graph.loadGraph(sender);
}

void Worker::procedureLoadValue(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GValue, move(msg));
		};
	graph.loadValue(sender);
}

void Worker::procedureLoadDelta(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GDelta, move(msg));
		};
	graph.loadDelta(sender);
}

void Worker::procedureBuildINCache(){
	std::function<void(const int, std::string&)> sender = 
		[&](const int wid, std::string& msg){
			net->send(wm.wid2nid(wid), MType::GINCache, move(msg));
		};
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
				info.tag = MType::PApply;
				driver.pushData("", info);
			}
			if(last_term.elapseMS() > tms){
				info.tag = MType::PReport;
				driver.pushData("", info);
			}
		}else{ // wake up by termination singal
			break;
		}
	}
}

void Worker::procedureDumpResult(){
	graph.dumpResult();
}

