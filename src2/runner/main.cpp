#include <iostream>
#include <string>
#include "Option.h"
#include "application/AppBase.h"
#include "network/NetworkThread.h"
#include "Master.h"
#include "Worker.h"

using namespace std;

int main(int argc, char* argv[]){
	Option opt;
	if(!opt.parseInput(argc, argv)){
		cerr<<"Failed in parsing arguments"<<endl;
		return 1;
	}
	NetworkThread::Init(argc, argv);
	if(opt.show){

	}
	AppBase app = makeApplication(opt.app_name, opt.app_args,
		opt.sharder_args, opt.sharder_args);
	
	NetworkThread* net = NetworkThread::GetInstance();
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
