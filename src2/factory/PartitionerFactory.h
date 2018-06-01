#pragma once
#include "application/Partitioner.h"
#include "FactoryTemplate.h"
#include <string>

class PartitionerFactory
	: public FactoryTemplate<PartitionerBase>
{
public:
	using parent_t = FactoryTemplate<PartitionerBase>;

	static void init();
};
