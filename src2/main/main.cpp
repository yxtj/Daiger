#include "application/AppBase.h"
#include "network/NetworkThread.h"
#include "example/example_reg.h"
#include "factory/factory_reg.h"
#include "runner/Option.h"
#include "runner/Master.h"
#include "runner/Worker.h"
#include "logging/logging.h"
#include "util/Timer.h"
#include <iostream>
#include <string>

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

//INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){
	Timer tmr;
	// init option
	Option opt;
	if(!opt.parseInput(argc, argv)){
		return 1;
	}
	registerExamples();
	registerFactories();
	initLogger(argc, argv);
	// init network
	NetworkThread::Init(argc, argv);
	NetworkThread* net = NetworkThread::GetInstance();

	// init App
	AppBase app;
	try{
		app = makeApplication(opt.app_name, opt.app_args,
			opt.partitioner_args, opt.scheduler_args,
			opt.terminator_args, opt.prioritizer_args, net->size());
	}catch(exception& e){
		LOG(ERROR)<<"Error in generating App via parameter: "<<e.what();
		return 2;
	}
	if(!app.check()){
		LOG(ERROR)<<"The application is not correctly setup.";
		return 3;
	}
	app.ptn->setParts(net->size());

	// check network size
	if(net->size() != 1 + opt.conf.nPart){
		// 1 master + <n> workers
		LOG(ERROR) << "The number of network instances (" << net->size()
			<< ") does not match required (" << 1 + opt.conf.nPart << ").";
		return 3;
	}

	if(opt.show && net->id() == 0){
		LOG(INFO)<<"Successfully initialized.\n"
			<<"\tPath graph: "<<opt.conf.path_graph<<"\n"
			<<"\tPath value: "<<opt.conf.path_value<<"\n"
			<<"\tPath delta: "<<opt.conf.path_delta<<"\n"
			<<"\tPath result: "<<opt.conf.path_result<<"\n"
			<<"\tPrefix graph: "<<opt.conf.prefix_graph<<"\n"
			<<"\tPrefix value: "<<opt.conf.prefix_value<<"\n"
			<<"\tPrefix delta: "<<opt.conf.prefix_delta<<"\n"
			<<"\tPrefix result: "<<opt.conf.prefix_result;
		LOG(INFO) << "Application: " << opt.app_name << "\n"
			<< "\targs: " << opt.app_args << "\n"
			<< "\tPartitioner: " << opt.partitioner_args << "\n"
			<< "\tScheduler: " << opt.scheduler_args << "\n"
			<< "\tTerminator: " << opt.terminator_args << "\n"
			<< "\tPrioritizer: " << opt.prioritizer_args;
		LOG(INFO)<<"Behavior parameters: \n"
			<<"\tBalanced loading: "<<opt.conf.balance_load<<"\n"
			<<"\tDo incremental: "<<opt.do_incremental<<"\n"
			<<"\tDo output: "<<opt.do_output<<"\t"<<"Sort result: "<<opt.conf.sort_result<<"\n"
			<<"\tAsynchronous: "<<opt.conf.async<<"\n"
			<<"\tCache-free: "<<opt.conf.cache_free;
		LOG(INFO) << "Runtime parameters: \n"
			<< "\tTimeout: " << opt.timeout << "\n"
			<< "\tApply: interval: " << opt.conf.apply_interval 
			<< "\tmin-ratio: "<<opt.conf.apply_min_portion << "\tmax-ratio: " << opt.conf.apply_max_portion << "\n"
			<< "\tSend: interval: " << opt.conf.send_interval
			<< "\tmin-size: " << opt.conf.send_min_size<< "\tmax-size: " << opt.conf.send_max_size << "\n"
			<< "\tProgress report interval: " << opt.conf.progress_interval<< "\n"
			<< "\tTermination max time: " << opt.conf.termination_max_time;
	}
	
	#ifndef NDEBUG
		if(net->id()==0){
			DLOG(INFO)<<"pause.";
			cin.get();
		}
	#endif
	// Run
	//return 0;
	if(net->id() == 0){ // master
		LOG(INFO)<<"starting master";
		Master m(app, opt);
		m.run();
	}else{ // worker
		LOG(INFO) << "starting worker " << net->id();
		Worker w(app, opt);
		w.run();
	}

	app.clear();
	if(net->id()==0){
		LOG(INFO)<<"Total Time used: "<<tmr.elapseSd();
	}
	// this should be the last one, because it invalids the pointer net.
	NetworkThread::Terminate();
	return 0;
}
