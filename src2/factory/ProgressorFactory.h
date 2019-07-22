#pragma once
#include "application/Progressor.h"
#include "FactoryTemplate.h"
#include <string>

class ProgressorFactory
	: public FactoryTemplate<ProgressorBase>
{
public:
	using parent_t = FactoryTemplate<ProgressorBase>;

	static void init();
};
