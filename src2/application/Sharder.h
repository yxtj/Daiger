#pragma once
#include "common/def.h"
#include <vector>
#include <string>

class SharderBase {
public:
	virtual ~SharderBase() = default;
	virtual void init(const std::vector<std::string>& args){}
	void setParts(const size_t n);
	// the the worker id of a node key
	virtual size_t owner(const id_t& id) = 0;
protected:
	size_t nWorker;
};

// -------- an example of a mod-based sharder --------

class SharderMod
	: public SharderBase
{
public:
	virtual size_t owner(const id_t& id);
};
