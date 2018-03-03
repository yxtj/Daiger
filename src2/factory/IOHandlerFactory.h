#pragma once
#include "application/IOHandler.h"
#include "FactoryTemplate.h"
#include <string>

class IOHandlerFactory
	: public FactoryTemplate<IOHandlerBase>
{
public:
	using parent_t = FactoryTemplate<IOHandlerBase>;

	static void init();
};
