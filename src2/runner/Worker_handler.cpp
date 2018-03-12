#include "Worker.h"
#include "msg/MType.h"
#include "msg/messages.h"
#include "network/NetworkThread.h"
#include "serial/serialization.h"
#include "runner_helpers.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <iostream>

using namespace std;

// register helpers
Worker::callback_t Worker::localCBBinder(
	void (Worker::*fp)(const std::string&, const RPCInfo&))
{
	return bind(fp, this, placeholders::_1, placeholders::_2);
}

void Worker::registerHandlers() {
	int nw=opt.conf.nPart;
	ReplyHandler::ConditionType EACH_ONE=ReplyHandler::EACH_ONE;
	
	// part 1: message handler
	regDSPProcess(MType::CReply, localCBBinder(&Worker::handleReply));
	regDSPProcess(MType::CRegister, localCBBinder(&Worker::handleRegister));
	regDSPProcess(MType::CWorkers, localCBBinder(&Worker::handleWorkers));
	regDSPProcess(MType::CShutdown, localCBBinder(&Worker::handleShutdown));
	regDSPImmediate(MType::CTerminate, localCBBinder(&Worker::handleTerminate));

	regDSPProcess(MType::CProcedure, localCBBinder(&Worker::handleProcedure));
	regDSPProcess(MType::CFinish, localCBBinder(&Worker::handleFinish));	

	regDSPProcess(MType::GNode, localCBBinder(&Worker::handleGNode));
	regDSPProcess(MType::GValue, localCBBinder(&Worker::handleGValue));
	regDSPProcess(MType::GDelta, localCBBinder(&Worker::handleGDelta));
	regDSPProcess(MType::GINCache, localCBBinder(&Worker::handleINCache));

	regDSPProcess(MType::VUpdate, localCBBinder(&Worker::handleVUpdate));
	regDSPProcess(MType::VRequest, localCBBinder(&Worker::handleVRequest));
	regDSPProcess(MType::VReply, localCBBinder(&Worker::handleVReply));

	// part 2: reply handler:
	//type 1: called by handleReply() directly

	//type 2: called by specific functions (handlers)
	// by handlerRegisterWorker()
	addRPHAnySU(MType::CWorkers, su_worker);
}

void Worker::handleReply(const std::string& d, const RPCInfo& info) {
    int type = deserialize<int>(d);
	int source = wm.nid2wid(info.source);
    rph.input(type, source);
}

void Worker::handleRegister(const std::string& d, const RPCInfo& info){
	int nid = deserialize<int>(d);
	master_net_id = nid;
	net->send(master_net_id, MType::CRegister, net->id());
}

void Worker::handleWorkers(const std::string& d, const RPCInfo& info){
	vector<pair<int, int>> idmapping = deserialize<vector<pair<int, int>>>(d);
	for(auto& p : idmapping){
		wm.register_worker(p.first, p.second);
	}
	registerWorker();
	//rph.input(MType::CWorkers, master_net_id);
	sendReply(info);
}
void Worker::handleShutdown(const std::string& d, const RPCInfo& info){
	shutdownWorker();
}
void Worker::handleTerminate(const std::string& d, const RPCInfo& info){
	terminateWorker();
}

void Worker::handleClear(const std::string& d, const RPCInfo& info){
	clearMessages();
	sendReply(info);
}
void Worker::handleProcedure(const std::string& d, const RPCInfo& info){
	int pid = deserialize<int>(d);
	function<void()> fun;
	switch(pid){
		case ProcedureType::LoadGraph:
			fun = bind(&Worker::procedureLoadGraph, this); break;
		case ProcedureType::LoadValue:
			fun = bind(&Worker::procedureLoadValue, this); break;
		case ProcedureType::LoadDelta:
			fun = bind(&Worker::procedureLoadDelta, this); break;
		case ProcedureType::BuildINCache:
			fun = bind(&Worker::procedureBuildINCache, this); break;
		case ProcedureType::Update:
			fun = bind(&Worker::procedureUpdate, this); break;
		case ProcedureType::DumpResult:
			fun = bind(&Worker::procedureDumpResult, this); break;
		default:
			cerr<<"Wrong Procedure ID."<<endl;
	}
	tprcd = thread(fun);
	sendReply(info);
}
void Worker::handleFinish(const std::string& d, const RPCInfo& info){
	net->flush();
	// TODO: abandon unprocessed procedure-related messages.
	// TODO: clear the resources for this procedure (if something left).
	tprcd.join();
	sendReply(info);
}

void Worker::handleGNode(const std::string& d, const RPCInfo& info){
	graph.loadGraphPiece(d);
}
void Worker::handleGValue(const std::string& d, const RPCInfo& info){
	graph.loadValuePiece(d);
}
void Worker::handleGDelta(const std::string& d, const RPCInfo& info){
	graph.loadDeltaPiece(d);
}

void Worker::handleINCache(const std::string& d, const RPCInfo& info){
	graph.takeINCache(d);
}

void Worker::handleVUpdate(const std::string& d, const RPCInfo& info){
	graph.msgUpdate(d);
}
void Worker::handleVRequest(const std::string& d, const RPCInfo& info){
	graph.msgRequest(d);
}
void Worker::handleVReply(const std::string& d, const RPCInfo& info){
	graph.msgReply(d);
}


