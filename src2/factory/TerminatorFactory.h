#pragma once
#include "application/Terminator.h"
#include "FactoryTemplate.h"
#include <string>

class TerminatorFactory
	: public FactoryTemplate<TerminatorBase>
{
public:
	using parent_t = FactoryTemplate<TerminatorBase>;

	static void init();
};
