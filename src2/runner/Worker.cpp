#include "Worker.h"
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
	// processed in handleRegister() and handleWorkers()
	//su_worker.wait();
	graph.init(wm.nid2wid(my_net_id), app.gh);
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

void Worker::procedureLoadGraph(){
	graph.loadGraph();
}

void Worker::procedureLoadValue(){
	graph.loadValue();
}

void Worker::procedureLoadDelta(){
	graph.loadDelta();
}

void Worker::procedureUpdate(){
	graph.update();
}

void Worker::procedureOutput(){
	graph.output();
}

