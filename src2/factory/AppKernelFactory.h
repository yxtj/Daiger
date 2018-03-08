#pragma once
#include "application/AppKernel.h"
#include "FactoryTemplate.h"
#include <string>

class AppKernelFactory
	: public FactoryTemplate<AppKernel>
{
public:
	using parent_t = FactoryTemplate<AppKernel>;

	static void init();
};


