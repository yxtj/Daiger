#include "Worker.h"
#include "network/NetworkThread.h"
#include <functional>
#include <chrono>

using namespace std;

Worker::Worker(const AppBase& app, Option& opt)
	: app(app), opt(move(opt)), running(false)
{
    net = NetworkThread::GetInstance();
    assert(net != nullptr);
    tmsg = thread(bind(&Worker::msgLoop, this));
	registerHandlers();
}

void Worker::start() {
	running = true;

}

void Worker::finish() {
	
	running = false;
    tmsg.join();
}

void Worker::sleep() {
	static auto d = chrono::duration<double>(0.01);
	this_thread::sleep_for(d);
}

void Worker::msgLoop() {
	// DLOG(INFO)<<"Message loop of master started";
	string data;
	RPCInfo info;
	info.dest = net->id();
	while(running){
		while(net->tryReadAny(data, &info.source, &info.tag)){
			// DVLOG(1)<<"Got a pkg from "<<info.source<<" to "<<info.dest<<", type "<<info.tag<<
			//		", queue length="<<driver.queSize();
			driver.pushData(data, info);
		}
		while(!driver.empty()){
//			DVLOG(1)<<"pop a message. driver left "<<driver_.queSize()<<" , net left "<<network_->unpicked_pkgs();
			driver.popData();
		}
		sleep();
	}
}
