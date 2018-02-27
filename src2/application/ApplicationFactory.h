#pragma once
#include "common/Kernel.h"
#include "FactoryTemplate.h"
#include <string>

class ApplicationFactory
	: public FactoryTemplate<KernelBase>
{
public:
	using parent_t = FactoryTemplate<KernelBase>;

	static void init();

	static KernelBase* generate(const std::string& name);
};
