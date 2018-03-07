#include "GlobalHolder.h"
#include "LocalHolder.hpp"
#include "RemoteHolder.hpp"
#include <vector>

using namespace std;

GlobalHolder::GlobalHolder(AppBase& app, const ConfData& conf)
	: app(app), conf(conf), impl(nullptr)
{
}
GlobalHolder::~GlobalHolder(){
	delete impl;
}

void GlobalHolder::loadGraph(){

}
void GlobalHolder::loadValue(){

}
void GlobalHolder::loadDelta(){

}
void GlobalHolder::update(){

}
void GlobalHolder::output(){
	
}

// --------

struct GlobalHolderImpl {

};

