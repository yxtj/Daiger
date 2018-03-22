#include "Runner.h"
#include "network/NetworkThread.h"
#include "serial/serialization.h"
#include "msg/MType.h"
#include "runner_helpers.h"
#include "logging/logging.h"
#include <cassert>
#include <chrono>
#include <functional>

using namespace std;
using namespace std::placeholders;

Runner::Runner(AppBase& app, Option& opt)
	: app(app), opt(move(opt)), timeout(opt.timeout), running(false)
{
    net = NetworkThread::GetInstance();
    assert(net != nullptr);
}

void Runner::sleep() {
	static auto d = chrono::duration<double>(0.01);
	this_thread::sleep_for(d);
}

void Runner::startMsgLoop(const std::string& name){
	running = true;
    tmsg = thread(bind(&Runner::msgLoop, this, name));
}
void Runner::stopMsgLoop(){
	running = false;
    tmsg.join();
}
void Runner::msgPausePush(){
	msg_do_push = false;
}
void Runner::msgPausePop(){
	msg_do_pop = false;
}
void Runner::msgResumePush(){
	msg_do_push = true;
}
void Runner::msgResumePop(){
	msg_do_push = true;
}
void Runner::msgLoop(const std::string& name) {
	DLOG(INFO)<<"Message loop started on "<<name;
	if(!name.empty()){
		setLogThreadName(name);
	}
	string data;
	RPCInfo info;
	info.dest = net->id();
	while(running){
		int n = 16; // prevent spending too much time in pushing but never popping 
		while(msg_do_push && n-->=0 && net->tryReadAny(data, &info.source, &info.tag)){
			DLOG(DEBUG)<<"Get "<<info.source<<" -> "<<info.dest<<", type "<<info.tag
				<<", queue: "<<driver.queSize()<<", net: "<<net->unpicked_pkgs();
			driver.pushData(data, info);
		}
		while(msg_do_pop && !driver.empty()){
			#ifndef NDEBUG
			auto& info = driver.front().second;
			DLOG(DEBUG)<<"Pop "<<info.source<<" -> "<<info.dest<<", type "<<info.tag
				<<", queue: "<<driver.queSize()<<", net: "<<net->unpicked_pkgs();
			#endif
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
void Runner::addRPHEachSU(const int type, SyncUnit& su){
	addRPHEach(type, bind(&SyncUnit::notify, &su), opt.conf.nPart, false);
}
void Runner::addRPHAny(
    const int type, std::function<void()> fun, const bool newThread)
{
    rph.addType(type,
        ReplyHandler::condFactory(ReplyHandler::ANY_ONE),
        fun, newThread);
	rph.activateType(type);
}
void Runner::addRPHAnySU(const int type, SyncUnit& su)
{
    addRPHAny(type, bind(&SyncUnit::notify, &su), false);
}

void Runner::sendReply(const RPCInfo& info){
	net->send(info.source, MType::CReply, info.tag);
}
