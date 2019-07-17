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
	if(opt.do_incremental) {
		procedureLoadValue();
		if(app.needInNeighbor){
			procedureBuildINList();
		}
		if(app.opt->is_selective()){
			procedureBuildINCache(); // necessary for both cache-base and cache-free cases
			procedureRebuildStructure(); // clear in-neighbor cache if cache-free
			procedureLoadDelta();
		}else{
			procedureLoadDelta();
			if(!opt.conf.cache_free){
				procedureBuildINCache();
			}
		}
	}
	procedureGenInitMsg();
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
	Timer tmr;
	while(!app.tmt->check_term()){
		su_term.wait();
		su_term.reset();
		if(VLOG_IS_ON(1)){
			auto s = app.tmt->state();
			auto d = app.tmt->difference();
			VLOG(1)<<"Time: "<<tmr.elapseSd()<<" current progress: ("<<s.first<<","<<s.second
				<<") improvement: ("<<d.first<<","<<d.second<<")";
		}
		if(tmr.elapseSd() > opt.term_time){
			VLOG(1) << "update timeout";
			break;
		}
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
		LOG(FATAL)<<"Timeout in registering workers";
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

	tmr_procedure.restart();
	DLOG(INFO)<<"starting new procedure: "<<pid;
	rph.resetTypeCondition(MType::CProcedure);
	su_procedure.reset();
	net->broadcast(MType::CProcedure, pid);
	su_procedure.wait();
	DLOG(INFO)<<"started new procedure: "<<pid;
}

void Master::finishProcedure(const int pid){
	DLOG(INFO)<<"waiting for finishing procedure: "<<pid;
	rph.resetTypeCondition(MType::CFinish);
	su_procedure.reset();
	net->broadcast(MType::CFinish, my_net_id);
	su_procedure.wait();
	DLOG(INFO)<<"finished procedure: "<<pid;
	LOG(INFO)<<"Time used: "<<tmr_procedure.elapseSd();
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
	LOG(INFO)<<"Starting loading graph.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish loading graph.";
}

void Master::procedureLoadValue(){
	cpid = ProcedureType::LoadValue;
	LOG(INFO)<<"Starting loading value.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish loading value.";
}

void Master::procedureLoadDelta(){
	cpid = ProcedureType::LoadDelta;
	LOG(INFO)<<"Starting loading delta.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish loading delta.";
}

void Master::procedureBuildINList(){
	cpid = ProcedureType::BuildINList;
	LOG(INFO) << "Starting building in-neighbor list.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO) << "Finish building in-neighbor list.";
}

void Master::procedureBuildINCache(){
	cpid = ProcedureType::BuildINCache;
	LOG(INFO)<<"Starting building in-neighbor cache.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish building in-neighbor cache.";
}

void Master::procedureRebuildStructure(){
	cpid = ProcedureType::RebuildStructure;
	LOG(INFO)<<"Starting reconstructing source structure.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish reconstructing source structure.";
}

void Master::procedureGenInitMsg(){
	cpid = ProcedureType::GenInitMsg;
	LOG(INFO)<<"Starting generating initial messages.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish generating initial messages.";
}

void Master::procedureUpdate(){
	cpid = ProcedureType::Update;
	LOG(INFO)<<"Starting updating.";
	startProcedure(cpid);
	terminationCheck();
	finishProcedure(cpid);
	LOG(INFO)<<"Finish updating.";
}

void Master::procedureDumpResult(){
	cpid = ProcedureType::DumpResult;
	LOG(INFO)<<"Starting damping.";
	startProcedure(cpid);
	finishProcedure(cpid);
	LOG(INFO)<<"Finish damping.";
}

