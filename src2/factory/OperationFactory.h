#pragma once
#include "application/Operation.h"
#include "FactoryTemplate.h"
#include <string>

class OperationFactory
	: public FactoryTemplate<OperationBase>
{
public:
	using parent_t = FactoryTemplate<OperationBase>;

	static void init();
};
