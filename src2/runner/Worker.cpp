#include "Worker.h"
#include "network/NetworkThread.h"
#include <functional>
#include <chrono>
#include <iostream>

using namespace std;

Worker::Worker(AppBase& app, Option& opt)
	: Runner(app, opt), graph(app, opt.conf)
{
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
	// processed by handleRegister() and handleWorkers()
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
	// TODO: wait until all incomming messages are processed or ignored.
	while(!driver.empty()){
		sleep();
		sleep();
	}
	net->flush();
}

void Worker::procedureLoadGraph(){
	holder.loadGraph();
}

void Worker::procedureLoadValue(){
	holder.loadValue();
}

void Worker::procedureLoadDelta(){
	holder.loadDelta();
}

void Worker::procedureUpdate(){
	holder.update();
}

void Worker::procedureOutput(){
	holder.output();
}

