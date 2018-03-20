#include "Master.h"
#include "network/NetworkThread.h"
#include "msg/MType.h"
#include "msg/messages.h"
#include <functional>
#include <vector>
#include <iostream>

using namespace std;

Master::Master(AppBase& app, Option& opt)
	: Runner(app, opt)
{
}

void Master::run() {
	registerHandlers();
	startMsgLoop();
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
    terminateWorker();
	stopMsgLoop();
    tmsg.join();
}

int Master::assignWid(const int nid){
	return nid - 1;
}

void Master::threadProgress(){
	while(!app.tmt->check_term()){
		su_term.wait();
	}
}
void Master::updateProgress(const int wid, const std::pair<double, size_t>& report){
	app.tmt->update_report(wid, report);
	rph.input(MType::PReport, wid);
}

void Master::registerWorker(){
	su_regw.reset();
	net->broadcast(MType::CRegister, net->id());
	// next in handleRegister
	if(!su_regw.wait_for(timeout)){
		cerr<<"Timeout in registering workers"<<endl;
		exit(1);
	}
	cout<<"All workers are registered"<<endl;
}

void Master::shutdownWorker(){
	su_procedure.reset();
	net->broadcast(MType::CShutdown, net->id());
	su_procedure.wait();
}

void Master::terminateWorker(){
	net->broadcast(MType::CTerminate, net->id());
}

void Master::startProcedure(const int pid){
	su_procedure.reset();
	net->broadcast(MType::CClear, net->id());
	su_procedure.wait();

	su_procedure.reset();
	net->broadcast(MType::CProcedure, pid);
	su_procedure.wait();
}

void Master::finishProcedure(const int pid){
	su_procedure.reset();
	net->broadcast(MType::CFinish, net->id());
	su_procedure.wait();
}

void Master::procedureInit(){
	cpid = ProcedureType::ShareWorkers;
	startProcedure(cpid);
	vector<pair<int, int>> winfo; // nid -> wid
	winfo.reserve(opt.conf.nPart);
	for(auto& w : wm.cont){
		winfo.emplace_back(w.first, w.second.wid);
	}
	su_regw.reset();
	net->broadcast(MType::CWorkers, winfo);
	// notified by handleReply()
	su_regw.wait();
	cout<<"Worker information is shared."<<endl;
	finishProcedure(cpid);
}

void Master::procedureLoadGraph(){
	cpid = ProcedureType::LoadGraph;
	startProcedure(cpid);
	cout<<"Starting loading graph."<<endl;
	finishProcedure(cpid);
	cout<<"Finish loading graph."<<endl;
}

void Master::procedureLoadValue(){
	cpid = ProcedureType::LoadValue;
	startProcedure(cpid);
	cout<<"Starting loading value."<<endl;
	finishProcedure(cpid);
	cout<<"Finish loading value."<<endl;
}

void Master::procedureLoadDelta(){
	cpid = ProcedureType::LoadDelta;
	startProcedure(cpid);
	cout<<"Starting loading delta."<<endl;
	finishProcedure(cpid);
	cout<<"Finish loading delta."<<endl;
}

void Master::procedureBuildINCache(){
	cpid = ProcedureType::BuildINCache;
	startProcedure(cpid);
	cout<<"Starting building in-neighbor cache."<<endl;
	finishProcedure(cpid);
	cout<<"Finish building in-neighbor cache."<<endl;
}

void Master::procedureUpdate(){
	cpid = ProcedureType::Update;
	startProcedure(cpid);
	cout<<"Starting updating."<<endl;
	thread tp(bind(&Master::threadProgress, this));
	tp.join();
	finishProcedure(cpid);
	cout<<"Finish updating."<<endl;
}

void Master::procedureDumpResult(){
	cpid = ProcedureType::DumpResult;
	startProcedure(cpid);
	cout<<"Starting damping."<<endl;
	finishProcedure(cpid);
	cout<<"Finish damping."<<endl;
}

