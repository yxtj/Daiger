#include <iostream>
#include <string>
#include "Option.h"
#include "network/NetworkThread.h"

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

	NetworkThread* net = NetworkThread::GetInstance();
	if(net->id() == 0){ // master

	}else{ // worker
		
	}

	return 0;
}
