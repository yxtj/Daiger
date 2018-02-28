#pragma once
#include "common/def.h"
#include <vector>
#include <string>

class SharderBase {
public:
	virtual void parse(const size_t nworkers, const std::vector<std::string>& arg){};
	// the the worker id of a node key
	virtual size_t owner(const key_t& id) = 0;
};

class SharderMod
	: public SharderBase
{
public:
	virtual void parse(const size_t nworkers, const std::vector<std::string>& arg);
	virtual size_t owner(const key_t& id);
private:
	size_t nworkers;
};
