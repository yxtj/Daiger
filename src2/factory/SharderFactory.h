#pragma once
#include "application/Sharder.h"
#include "FactoryTemplate.h"
#include <string>

class SharderFactory
	: public FactoryTemplate<SharderBase>
{
public:
	using parent_t = FactoryTemplate<SharderBase>;

	static void init();
};
