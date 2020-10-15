#include "factory_reg.h"
#include "PartitionerFactory.h"
#include "SchedulerFactory.h"
#include "TerminatorFactory.h"
#include "PrioritizerFactory.h"

void registerFactories(){
	PartitionerFactory::init();
	SchedulerFactory::init();
	TerminatorFactory::init();
	PrioritizerFactory::init();
}
