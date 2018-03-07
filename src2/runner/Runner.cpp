#include "Runner.h"
#include "network/NetworkThread.h"
#include "serial/serialization.h"
#include "msg/MType.h"
#include "runner_helpers.h"
#include <cassert>
#include <chrono>
#include <functional>

using namespace std;
using namespace std::placeholders;

Runner::Runner(const AppBase& app, Option& opt)
	: app(app), opt(move(opt)), running(false), timeout(opt.timeout)
{
    net = NetworkThread::GetInstance();
    assert(net != nullptr);
}

void Runner::sleep() {
	static auto d = chrono::duration<double>(0.01);
	this_thread::sleep_for(d);
}

void Runner::startMsgLoop(){
	running = true;
    tmsg = thread(bind(&Runner::msgLoop, this));
}
void Runner::stopMsgLoop(){
	running = false;
    tmsg.join();
}

void Runner::msgLoop() {
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

// register helpers
void Runner::regDSPImmediate(const int type, callback_t fp) {
    //driver.registerImmediateHandler(type, bind(fp, this, _1, _2));
	driver.registerImmediateHandler(type, fp);
}
void Runner::regDSPProcess(const int type, callback_t fp) {
    //driver.registerProcessHandler(type, bind(fp, this, _1, _2));
	driver.registerProcessHandler(type, fp);
}
void Runner::regDSPDefault(callback_t fp) {
    //driver.registerDefaultOutHandler(bind(fp, this, _1, _2));
	driver.registerDefaultOutHandler(fp);
}
void Runner::addRPHEach(
    const int type, std::function<void()> fun, const int n, const bool newThread)
{
    rph.addType(type,
        ReplyHandler::condFactory(ReplyHandler::EACH_ONE, n),
        fun, newThread);
	rph.activateType(type);
}
void addRPHEachSU(const int type, SyncUnit& su){
	addRPHEach(type, bind(&SyncUnit::notify, &su), opt.nPart, false);
}

void Runner::sendReply(const RPCInfo& info){
	net->send(info.source, MType::CReply, info.tag);
}