#include "Master.h"
#include "network/NetworkThread.h"
#include "msg/MType.h"
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
	idmapping.reserve(opt.nPart);
	for(auto& w : wm.cont){
		idmapping.emplace_back(w.first, w.second.wid);
	}
	su_regw.reset();
	net->broadcast(MType::CWorkers, idmapping);
	if(!su_regw.wait_for(timeout)){
		cerr<<"Timeout"<<endl;
		exit(1);
	}
	cout<<"All workers are registed"<<endl;
}

void Master::terminateWorker(){
	su_procedure.reset();

	su_procedure.wait();
}

void Master::procedureLoadGraph(){
	su_procedure.reset();

	su_procedure.wait();
}

void Master::procedureLoadValue(){
	su_procedure.reset();

	su_procedure.wait();
}

void Master::procedureLoadDelta(){
	su_procedure.reset();

	su_procedure.wait();
}

void Master::procedureUpdate(){
	su_procedure.reset();

	su_procedure.wait();
}

void Master::procedureOutput(){
	su_procedure.reset();

	su_procedure.wait();
}

