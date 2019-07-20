#pragma once
#include "common/def.h"
#include <vector>
#include <string>

class PartitionerBase {
public:
	virtual ~PartitionerBase() = default;
	virtual void init(const std::vector<std::string>& args){}
	void setParts(const size_t n);
	// the the worker id of a node key
	virtual int owner(const id_t& id) const = 0;
protected:
	size_t nWorker;
};

// -------- an example of a mod-based partitioner --------

class PartitionerMod
	: public PartitionerBase
{
public:
	virtual int owner(const id_t& id) const;
};
