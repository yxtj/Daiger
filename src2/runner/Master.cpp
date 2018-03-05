#include "Master.h"
#include "network/NetworkThread.h"
#include <functional>
#include <chrono>

using namespace std;

Master::Master(const AppBase& app, Option& opt)
	: app(app), opt(move(opt)), running(false)
{
    net = NetworkThread::GetInstance();
    assert(net != nullptr);
    tmsg = thread(bind(&Master::msgLoop, this));
	registerHandlers();
}

void Master::start() {
	running = true;
    registerWorkers();

    procedureLoadGraph();
    if (opt.do_incremental) {
        procedureLoadValue();
        procedureLoadDelta();
    }
    procedureUpdate();
    if (opt.do_output) {
        procedureOutput();
    }
}

void Master::finish() {
    terminateWorkers();
	running = false;
    tmsg.join();
}

void Master::sleep() {
	static auto d = chrono::duration<double>(0.01);
	this_thread::sleep_for(d);
}

void Master::msgLoop() {
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
