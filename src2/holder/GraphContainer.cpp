#include "GraphContainer.h"
#include "GlobalHolder.hpp"
#include <vector>

using namespace std;

GraphContainer::GraphContainer(AppBase& app, const ConfData& conf)
	: app(app), conf(conf), holder(nullptr)
{
}
GraphContainer::~GraphContainer(){
	delete holder;
}

void GraphContainer::loadGraph(){

}
void GraphContainer::loadValue(){

}
void GraphContainer::loadDelta(){

}
void GraphContainer::update(){

}
void GraphContainer::output(){
	
}

// --------

