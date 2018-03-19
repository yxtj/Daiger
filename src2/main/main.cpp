#include <iostream>
#include <string>
#include "application/AppBase.h"
#include "network/NetworkThread.h"
#include "example/example_reg.h"
#include "factory/factory_reg.h"
#include "runner/Option.h"
#include "runner/Master.h"
#include "runner/Worker.h"

using namespace std;

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& vec){
	os<<"{ ";
	for(auto& v : vec){
		os<<v<<" ";
	}
	os<<"}";
	return os;
}

int main(int argc, char* argv[]){
	// init option
	Option opt;
	if(!opt.parseInput(argc, argv)){
		return 1;
	}
	registerExamples();
	registerFactories();
	// init App
	AppBase app = makeApplication(opt.app_name, opt.app_args,
		opt.sharder_args, opt.scheduler_args);
	if(!app.check()){
		cerr<<"The application is not correctly setup."<<endl;
		return 2;
	}
	// init network
	NetworkThread::Init(argc, argv);
	NetworkThread* net = NetworkThread::GetInstance();
	if(net->size() != 1 + opt.conf.nPart){
		// 1 master + <n> workers
		cerr<<"The number of network instances ("<<net->size()
			<<") does not match required ("<<1 + opt.conf.nPart<<")."<<endl;
		return 3;
	}
	app.shd->setParts(net->size());

	if(opt.show && net->id() == 0){
		cout<<"Successfully initialized.\n"
			<<"\tPath graph: "<<opt.conf.path_graph<<"\n"
			<<"\tPath value: "<<opt.conf.path_value<<"\n"
			<<"\tPath delta: "<<opt.conf.path_delta<<"\n"
			<<"\tPath result: "<<opt.conf.path_result<<"\n"
			<<"\tPrefix graph: "<<opt.conf.prefix_graph<<"\n"
			<<"\tPrefix value: "<<opt.conf.prefix_value<<"\n"
			<<"\tPrefix delta: "<<opt.conf.prefix_delta<<"\n"
			<<"\tPrefix result: "<<opt.conf.prefix_result<<"\n";
		cout<<"Application: "<<opt.app_name<<"\n"
			<<"\targs: "<<opt.app_args<<"\n"
			<<"\tPartitioner: "<<opt.sharder_args<<"\n"
			<<"\tScheduler: "<<opt.scheduler_args<<"\n";
	}
	
	#ifndef NDEBUG
		if(net->id()==0){
			cerr<<"pause";
			cerr.flush();
			cout<<cin.get()<<endl;
		}
	#endif
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

	NetworkThread::Terminate();
	return 0;
}
