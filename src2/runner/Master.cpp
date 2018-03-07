#include "Master.h"
#include "network/NetworkThread.h"
#include "msg/MType.h"
#include "msg/messages.h"
#include <functional>
#include <vector>
#include <iostream>

using namespace std;

Master::Master(const AppBase& app, Option& opt)
	: Runner(app, opt)
{
}

void Master::start() {
	registerHandlers();
	startMsgLoop();
    registerWorker();

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
    terminateWorker();
	stopMsgLoop();
    tmsg.join();
}

int assignWid(const int nid){
	return nid - 1;
}

void Master::registerWorker(){
	su_regw.reset();
	net->broadcast(MType::CRegister, net->id());
	if(!su_regw.wait_for(timeout)){
	}
	vector<pair<int, int>> idmapping; // nid -> wid
	idmapping.reserve(opt.conf.nPart);
	for(auto& w : wm.cont){
		idmapping.emplace_back(w.first, w.second.wid);
	}
	su_regw.reset();
	net->broadcast(MType::CWorkers, idmapping);
	if(!su_regw.wait_for(timeout)){
		cerr<<"Timeout"<<endl;
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

void Master::procedureLoadGraph(){
	cpid = ProcedureType::LoadGraph;
	startProcedure(cpid);
	finishProcedure(cpid);
}

void Master::procedureLoadValue(){
	cpid = ProcedureType::LoadValue;
	startProcedure(cpid);
	finishProcedure(cpid);
}

void Master::procedureLoadDelta(){
	cpid = ProcedureType::LoadDelta;
	startProcedure(cpid);
	finishProcedure(cpid);
}

void Master::procedureUpdate(){
	cpid = ProcedureType::Update;
	startProcedure(cpid);
	// TODO: control progress report and termination check
	finishProcedure(cpid);
}

void Master::procedureOutput(){
	cpid = ProcedureType::Output;
	startProcedure(cpid);
	finishProcedure(cpid);
}

