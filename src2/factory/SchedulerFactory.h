#pragma once
#include "application/Scheduler.h"
#include "FactoryTemplate.h"
#include <string>

class SchedulerFactory
	: public FactoryTemplate<SchedulerBase>
{
public:
	using parent_t = FactoryTemplate<SchedulerBase>;

	static void init();
};
