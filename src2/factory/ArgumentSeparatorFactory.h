#pragma once
#include "application/ArgumentSeparator.h"
#include "FactoryTemplate.h"
#include <string>

class ArgumentSeparatorFactory
	: public FactoryTemplate<ArgumentSeparator>
{
public:
	using parent_t = FactoryTemplate<ArgumentSeparator>;

	static void init();
};
