#pragma once
#include "common/def.h"
#include <vector>
#include <string>

class SharderBase {
public:
	virtual void init(const size_t n_workers, const std::vector<std::string>& arg){};
	// the the worker id of a node key
	virtual size_t owner(const key_t& id) = 0;
protected:
	size_t nWorker;
};

// -------- an example of a mod-based sharder --------

class SharderMod
	: public SharderBase
{
public:
	virtual void init(const size_t n_workers, const std::vector<std::string>& arg);
	virtual size_t owner(const key_t& id);
};
