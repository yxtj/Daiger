#include "Worker.h"
#include "msg/MType.h"
#include "msg/messages.h"
//#include "network/NetworkThread.h"
#include "serial/serialization.h"
#include "runner_helpers.h"
#include "logging/logging.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <iostream>

using namespace std;

// register helpers
Worker::callback_t Worker::localCBBinder(
	void (Worker::*fp)(std::string&, const RPCInfo&))
{
	return bind(fp, this, placeholders::_1, placeholders::_2);
}

void Worker::registerHandlers() {
	//int nw=opt.conf.nPart;
	//ReplyHandler::ConditionType EACH_ONE=ReplyHandler::EACH_ONE;
	
	// part 1: message handler
	regDSPProcess(MType::CReply, localCBBinder(&Worker::handleReply));
	regDSPProcess(MType::COnline, localCBBinder(&Worker::handleOnline));
	regDSPProcess(MType::CRegister, localCBBinder(&Worker::handleRegister));
	regDSPProcess(MType::CWorkers, localCBBinder(&Worker::handleWorkers));
	regDSPProcess(MType::CShutdown, localCBBinder(&Worker::handleShutdown));
	regDSPImmediate(MType::CTerminate, localCBBinder(&Worker::handleTerminate));

	regDSPProcess(MType::CProcedure, localCBBinder(&Worker::handleProcedure));
	regDSPProcess(MType::CFinish, localCBBinder(&Worker::handleFinish));
	regDSPProcess(MType::CClear, localCBBinder(&Worker::handleClear));

	regDSPProcess(MType::GNode, localCBBinder(&Worker::handleGNode));
	regDSPProcess(MType::GValue, localCBBinder(&Worker::handleGValue));
	regDSPProcess(MType::GDelta, localCBBinder(&Worker::handleGDelta));
	regDSPProcess(MType::GINCache, localCBBinder(&Worker::handleINCache));

	regDSPProcess(MType::VUpdate, localCBBinder(&Worker::handleVUpdate));
	regDSPProcess(MType::VRequest, localCBBinder(&Worker::handleVRequest));
	regDSPProcess(MType::VReply, localCBBinder(&Worker::handleVReply));

	//regDSPProcess(MType::PApply, localCBBinder(&Worker::handlePApply));
	//regDSPProcess(MType::PSend, localCBBinder(&Worker::handlePSend));
	//regDSPProcess(MType::PReport, localCBBinder(&Worker::handlePReport));
	regDSPProcess(MType::PFinish, localCBBinder(&Worker::handlePFinish));

	// part 2: reply handler:
	//type 1: called by handleReply() directly
	addRPHAnySU(MType::CRegister, su_regw);

	//type 2: called by specific functions (handlers)
	// by handlerRegisterWorker()
}

void Worker::handleReply(std::string& d, const RPCInfo& info) {
    int type = deserialize<int>(d);
	pair<bool, int> source = wm.nidtrans(info.source);
    rph.input(type, source.second);
}

void Worker::handleOnline(std::string& d, const RPCInfo& info){
	int nid = deserialize<int>(d);
	master_net_id = nid;
	// sendReply(info);
	DLOG(DEBUG)<<"got master id";
	su_master.notify(); // notify registerWorker()
}

void Worker::handleRegister(std::string& d, const RPCInfo& info){
	int nid = deserialize<int>(d);
	master_net_id = nid;
	LOG(DEBUG)<<"got master id";
	su_master.notify(); // notify registerWorker()
}

void Worker::handleWorkers(std::string& d, const RPCInfo& info){
	vector<pair<int, int>> winfo = deserialize<vector<pair<int, int>>>(d);
	storeWorkerInfo(winfo);
	sendReply(info);
}
void Worker::handleShutdown(std::string& d, const RPCInfo& info){
	shutdownWorker();
	sendReply(info);
}
void Worker::handleTerminate(std::string& d, const RPCInfo& info){
	terminateWorker();
}

void Worker::handleClear(std::string& d, const RPCInfo& info){
	if(tprcd.joinable())
		tprcd.join();
	// pass info as an value copy to keep it until it is used
	tprcd = thread([&](RPCInfo info){
		// TODO: clear the resources for this procedure (if something left).
		// TODO: abandon unprocessed procedure-related messages.
		clearMessages();
		sendReply(info);
	}, info);
}
void Worker::handleProcedure(std::string& d, const RPCInfo& info){
	int pid = deserialize<int>(d);
	function<void()> fun;
	switch(pid){
		case ProcedureType::ShareWorkers:
			fun = bind(&Worker::procedureInit, this); break;
		case ProcedureType::LoadGraph:
			fun = bind(&Worker::procedureLoadGraph, this); break;
		case ProcedureType::LoadValue:
			fun = bind(&Worker::procedureLoadValue, this); break;
		case ProcedureType::BuildINList:
			fun = bind(&Worker::procedureBuildINList, this); break;
		case ProcedureType::BuildINCache:
			fun = bind(&Worker::procedureBuildINCache, this); break;
		case ProcedureType::RebuildStructure:
			fun = bind(&Worker::procedureRebuildStructure, this); break;
		case ProcedureType::LoadDelta:
			fun = bind(&Worker::procedureLoadDelta, this); break;
		case ProcedureType::GenInitMsg:
			fun = bind(&Worker::procedureGenInitMsg, this); break;
		case ProcedureType::Update:
			fun = bind(&Worker::procedureUpdate, this); break;
		case ProcedureType::DumpResult:
			fun = bind(&Worker::procedureDumpResult, this); break;
		default:
			cerr<<"Wrong Procedure ID."<<endl;
	}
	if(tprcd.joinable())
		tprcd.join();
	tprcd = thread(fun);
	sendReply(info);
}
void Worker::handleFinish(std::string& d, const RPCInfo& info){
	if(tprcd.joinable())
		tprcd.join();
	// pass info as an value copy to keep it until it is used
	tprcd = thread([&](RPCInfo info){
		// TODO: clear the resources for this procedure (if something left).
		// TODO: abandon unprocessed procedure-related messages.
		clearMessages();
		sendReply(info);
	}, info);
}

void Worker::handleGNode(std::string& d, const RPCInfo& info){
	graph.loadGraphPiece(d);
}
void Worker::handleGValue(std::string& d, const RPCInfo& info){
	graph.loadValuePiece(d);
}
void Worker::handleGDelta(std::string& d, const RPCInfo& info){
	graph.loadDeltaPiece(d);
}

void Worker::handleINCache(std::string& d, const RPCInfo& info){
	graph.takeINCache(d);
}

void Worker::handleVUpdate(std::string& d, const RPCInfo& info){
	//graph.msgUpdate(d);
	graph.pushMsg(GraphContainer::MsgType::Update, d);
}
void Worker::handleVRequest(std::string& d, const RPCInfo& info){
	//graph.msgRequest(d);
	graph.pushMsg(GraphContainer::MsgType::Request, d);
}
void Worker::handleVReply(std::string& d, const RPCInfo& info){
	//graph.msgReply(d);
	graph.pushMsg(GraphContainer::MsgType::Reply, d);
}

/*
void Worker::handlePApply(std::string& d, const RPCInfo& info){
	graph.apply();
}
void Worker::handlePSend(std::string& d, const RPCInfo& info){
	graph.send();
}
void Worker::handlePReport(std::string& d, const RPCInfo& info){
	reportProgress();
}
*/

void Worker::handlePFinish(std::string& d, const RPCInfo& info){
	//update_finish = true;
	//su_update.notify();
	graph.stop_update();
}
