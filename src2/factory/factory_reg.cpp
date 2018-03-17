#include "factory_reg.h"
#include "SharderFactory.h"
#include "SchedulerFactory.h"

void registerFactories(){
	SharderFactory::init();
	SchedulerFactory::init();
}
