#include "Worker.h"
#include "network/NetworkThread.h"
#include <functional>
#include <chrono>

using namespace std;

Worker::Worker(const AppBase& app, Option& opt)
	: Runner(app, opt)
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

void Worker::terminateWorker(){

}

void Worker::procedureLoadGraph(){
	
}

void Worker::procedureLoadValue(){
	
}

void Worker::procedureLoadDelta(){
	
}

void Worker::procedureUpdate(){
	
}

void Worker::procedureOutput(){
	
}

