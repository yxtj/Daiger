#include <iostream>
#include <string>
#include "application/AppBase.h"
#include "network/NetworkThread.h"
#include "example/example_reg.h"
#include "runner/Option.h"
#include "runner/Master.h"
#include "runner/Worker.h"

using namespace std;

int main(int argc, char* argv[]){
	// init option
	Option opt;
	if(!opt.parseInput(argc, argv)){
		cerr<<"Failed in parsing arguments."<<endl;
		return 1;
	}
	if(opt.show){

	}
	registerExamples();
	// init App
	AppBase app = makeApplication(opt.app_name, opt.app_args,
		opt.sharder_args, opt.sharder_args);
	if(!app.check()){
		cerr<<"The application is not correctly setup."<<endl;
		return 2;
	}
	// init network
	NetworkThread::Init(argc, argv);
	NetworkThread* net = NetworkThread::GetInstance();
	if(net->size() != 1 + opt.conf.nPart){
		// 1 master + <n> workers
		cerr<<"The number of network instances does not match the number of workers."<<endl;
		return 3;
	}
	app.shd->setParts(net->size());
	
	// Run
	if(net->id() == 0){ // master
		Master m(app, opt);
		m.start();
		m.finish();
	}else{ // worker
		Worker w(app, opt);
		w.start();
		w.finish();
	}

	return 0;
}
