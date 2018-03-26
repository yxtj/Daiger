#include "Master.h"
#include "network/NetworkThread.h"
#include "msg/MType.h"
#include "msg/messages.h"
#include "logging/logging.h"
#include <functional>
#include <vector>
#include <iostream>

using namespace std;

Master::Master(AppBase& app, Option& opt)
	: Runner(app, opt)
{
	my_net_id = net->id();
	setLogThreadName("M"+to_string(my_net_id));
}

void Master::run() {
	registerHandlers();
	startMsgLoop("M"+to_string(my_net_id)+"-MSG");
    registerWorker();

	procedureInit();
    procedureLoadGraph();
    if (opt.do_incremental) {
        procedureLoadValue();
        procedureLoadDelta();
		procedureBuildINCache();
    }
    procedureUpdate();
    if (opt.do_output) {
        procedureDumpResult();
    }

	// finish
    shutdownWorker();
	clearMessages();
	stopMsgLoop();
	LOG(INFO)<<"master stops";
}

int Master::assignWid(const int nid){
	return nid - 1;
}

void Master::terminationCheck(){
	while(!app.tmt->check_term()){
		su_term.wait();
	}
	VLOG(1)<<"update terminates";
	net->broadcast(MType::PFinish, my_net_id);
}
void Master::updateProgress(const int wid, const ProgressReport& report){
	app.tmt->update_report(wid, report);
	rph.input(MType::PReport, wid);
}

void Master::registerWorker(){
	su_regw.reset();
	net->broadcast(MType::COnline, my_net_id);
	// notified by handleRegister
	if(!su_regw.wait_for(timeout)){
		LOG(ERROR)<<"Timeout in registering workers";
		exit(1);
	}
	LOG(INFO)<<"All workers are registered";
}

void Master::shutdownWorker(){
	rph.resetTypeCondition(MType::CShutdown);
	su_procedure.reset();
	net->broadcast(MType::CShutdown, my_net_id);
	su_procedure.wait();
}

void Master::terminateWorker(){
	net->broadcast(MType::CTerminate, my_net_id);
}

void Master::startProcedure(const int pid){
	// DLOG(INFO)<<"clearing for new procedure: "<<pid;
	// rph.resetTypeCondition(MType::CClear);
	// su_procedure.reset();
	// net->broadcast(MType::CClear, my_net_id);
	// su_procedure.wait();

	DLOG(INFO)<<"starting new procedure: "<<pid;
	rph.resetTypeCondition(MType::CProcedure);
	su_procedure.reset();
	net->broadcast(MType::CProcedure, pid);
	su_procedure.wait();
	DLOG(INFO)<<"started new procedure: "<<pid;
}

void Master::finishProcedure(const int pid){
	DLOG(INFO)<<"finishing procedure: "<<pid;
	rph.resetTypeCondition(MType::CFinish);
	su_procedure.reset();
	net->broadcast(MType::CFinish, my_net_id);
	su_procedure.wait();
	DLOG(INFO)<<"finished procedure: "<<pid;
}

void Master::procedureInit(){
	cpid = ProcedureType::ShareWorkers;
	startProcedure(cpid);
	app.tmt->prepare_global_checker(opt.conf.nPart);
	vector<pair<int, int>> winfo; // nid -> wid
	winfo.reserve(opt.conf.nPart);
	for(auto& w : wm.cont){
		winfo.emplace_back(w.first, w.second.wid);
	}
	su_regw.reset();
	net->broadcast(MType::CWorkers, winfo);
	// notified by handleReply()
	su_regw.wait();
	LOG(INFO)<<"Worker information is shared.";
	finishProcedure(cpid);
}

void Master::procedureLoadGraph(){
	cpid = ProcedureType::LoadGraph;
	startProcedure(cpid);
	LOG(INFO)<<"Starting loading graph.";
	finishProcedure(cpid);
	LOG(INFO)<<"Finish loading graph.";
}

void Master::procedureLoadValue(){
	cpid = ProcedureType::LoadValue;
	startProcedure(cpid);
	LOG(INFO)<<"Starting loading value.";
	finishProcedure(cpid);
	LOG(INFO)<<"Finish loading value.";
}

void Master::procedureLoadDelta(){
	cpid = ProcedureType::LoadDelta;
	startProcedure(cpid);
	LOG(INFO)<<"Starting loading delta.";
	finishProcedure(cpid);
	LOG(INFO)<<"Finish loading delta.";
}

void Master::procedureBuildINCache(){
	cpid = ProcedureType::BuildINCache;
	startProcedure(cpid);
	LOG(INFO)<<"Starting building in-neighbor cache.";
	finishProcedure(cpid);
	LOG(INFO)<<"Finish building in-neighbor cache.";
}

void Master::procedureUpdate(){
	cpid = ProcedureType::Update;
	startProcedure(cpid);
	LOG(INFO)<<"Starting updating.";
	terminationCheck();
	finishProcedure(cpid);
	LOG(INFO)<<"Finish updating.";
}

void Master::procedureDumpResult(){
	cpid = ProcedureType::DumpResult;
	startProcedure(cpid);
	LOG(INFO)<<"Starting damping.";
	finishProcedure(cpid);
	LOG(INFO)<<"Finish damping.";
}

