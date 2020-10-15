#pragma once
#include "application/Prioritizer.h"
#include "FactoryTemplate.h"
#include <string>

class PrioritizerFactory
	: public FactoryTemplate<PrioritizerBase>
{
public:
	using parent_t = FactoryTemplate<PrioritizerBase>;

	static void init();

	// overwrite the generate funtion (throw an exception when called)
	static PrioritizerBase* generate(const std::string& name);

	// provide a typed generate function
	template <class V, class N>
	static PrioritizerBase* generate(const std::string& name){
		PrioritizerBase* p = nullptr;
		if(name == "none"){
			p = new PrioritizerNone<V, N>();
		} else if(name == "diff"){
			p = new PrioritizerDiff<V, N>();
		} else if(name == "value"){
			p = new PrioritizerValue<V, N>();
		} else if(name == "diff_deg"){
			p = new PrioritizerDiffODeg<V, N>();
		} else if(name == "value_deg"){
			p = new PrioritizerValueODeg<V, N>();
		}
		return p;
	}
};
