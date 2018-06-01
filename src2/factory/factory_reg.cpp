#include "factory_reg.h"
#include "PartitionerFactory.h"
#include "SchedulerFactory.h"

void registerFactories(){
	PartitionerFactory::init();
	SchedulerFactory::init();
}
