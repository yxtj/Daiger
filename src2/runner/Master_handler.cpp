#include "Master.h"
#include "msg/MType.h"
#include "network/NetworkThread.h"
#include "serial/serialization.h"
#include "runner_helpers.h"
#include "util/Timer.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <cassert>

using namespace std;

// register helpers
Master::callback_t Master::localCBBinder(
	void (Master::*fp)(const std::string&, const RPCInfo&))
{
	return bind(fp, this, placeholders::_1, placeholders::_2);
}

void Master::registerHandlers() {
	int nw=opt.conf.nPart;
	ReplyHandler::ConditionType EACH_ONE=ReplyHandler::EACH_ONE;
	
	// part 1: message handler
	regDSPProcess(MType::CReply, localCBBinder(&Master::handleReply));
	regDSPProcess(MType::CRegister, localCBBinder(&Master::handleRegister));
	regDSPProcess(MType::TReport, localCBBinder(&Master::handleProgressReport));

	// part 2: reply handler:
	//type 1: called by handleReply() directly
	addRPHEachSU(MType::CClear, su_procedure);
	addRPHEachSU(MType::CProcedure, su_procedure);
	addRPHEachSU(MType::CFinish, su_procedure);
	addRPHEachSU(MType::CShutdown, su_procedure);

	//type 2: called by specific functions (handlers)
	// by handlerRegisterWorker()
	addRPHEachSU(MType::CRegister, su_regw);
	// by handlerRegisterWorker()
	addRPHEachSU(MType::CWorkers, su_regw);
	// by handleProgressReport()
	addRPHEachSU(MType::TReport, su_term);
}

void Master::handleReply(const std::string& d, const RPCInfo& info) {
    int type = deserialize<int>(d);
	int source = wm.nid2wid(info.source);
    rph.input(type, source);
}

void Master::handleRegister(const std::string& d, const RPCInfo& info){
	// VLOG(1)<<"Registered worker from: " << info.source;
	int wid = assignWid(info.source);
	wm.register_worker(info.source, wid);
	rph.input(MType::CRegister, wid);
}

void Master::handleProgressReport(const std::string& d, const RPCInfo& info){
	pair<double, size_t> report = deserialize<pair<double, size_t> >(d);
	int wid = wm.nid2wid(info.source);
	app.tmt->update_report(wid, report);
	wm.update_report_time(info.source, Timer::Now());
	rph.input(MType::TReport, wid);
}

