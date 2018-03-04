#pragma once
#include "application/ArgumentSeparator.h"
#include "FactoryTemplate.h"
#include <string>

class ArgumentSeparatorFactory
	: public FactoryTemplate<ArgumentSpearator>
{
public:
	using parent_t = FactoryTemplate<ArgumentSpearator>;

	static void init();
};
