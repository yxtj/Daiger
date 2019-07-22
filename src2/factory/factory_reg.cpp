#include "factory_reg.h"
#include "PartitionerFactory.h"
#include "SchedulerFactory.h"
#include "TerminatorFactory.h"

void registerFactories(){
	PartitionerFactory::init();
	SchedulerFactory::init();
	TerminatorFactory::init();
}
