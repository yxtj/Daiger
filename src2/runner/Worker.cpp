#include "Worker.h"
#include "msg/MType.h"
#include "network/NetworkThread.h"
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

}

void Worker::finish() {
    terminateWorker();
	stopMsgLoop();
    tmsg.join();
}

void Worker::registerWorker(){
	// called by handleRegister()
	net->send(master_net_id, MType::CRegister, net->id());
}

void Worker::shutdownWorker(){

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
	graph.buildInNeighborCache(sender);
}

void Worker::procedureUpdate(){
	//graph.update();
}

void Worker::procedureDumpResult(){
	graph.dumpResult();
}

