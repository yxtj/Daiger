#pragma once
#include "common/Node.h"
#include <vector>
#include <string>
#include <cmath>

class PrioritizerBase{
public:
	virtual ~PrioritizerBase() = default;
	virtual void init(const std::vector<std::string>& args){};
};

template <typename V, typename N>
class Prioritizer
	: public PrioritizerBase
{
public:
	// get the priority of a single node
	virtual priority_t priority(const Node<V, N>& n) = 0;
};

// -------- common priority examples --------
template <typename V, typename N>

class PrioritizerNone
	: public Prioritizer<V, N>
{
public:
	virtual priority_t priority(const Node<V, N>& n){
		return static_cast<priority_t>(0.0);
	}
};

template <typename V, typename N>
class PrioritizerValue
	: public Prioritizer<V, N>
{
public:
	virtual priority_t priority(const Node<V, N>& n){
		return static_cast<priority_t>(n.u);
	}
};

template <typename V, typename N>
class PrioritizerValueODeg
	: public Prioritizer<V, N>
{
public:
	virtual priority_t priority(const Node<V, N>& n){
		return static_cast<priority_t>(n.u * n.onb.size());
	}
};

template <typename V, typename N>
class PrioritizerDiff
	: public Prioritizer<V, N>
{
public:
	virtual priority_t priority(const Node<V, N>& n){
		V p = std::abs<V>(n.u - n.v);
		return static_cast<priority_t>(p);
	}
};

template <typename V, typename N>
class PrioritizerDiffODeg
	: public Prioritizer<V, N>
{
public:
	virtual priority_t priority(const Node<V, N>& n){
		V p = std::abs<V>(n.u - n.v);
		return static_cast<priority_t>(p * n.onb.size());
	}
};
