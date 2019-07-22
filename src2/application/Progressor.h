#pragma once
#include "common/Node.h"
#include <vector>
#include <string>
#include <limits>

class ProgressorBase{
public:
	virtual ~ProgressorBase() = default;
	virtual void init(const std::vector<std::string>& args){};
};

template <typename V, typename N>
struct ProgressHelper;

template <typename V, typename N>
class Progressor
	: public ProgressorBase, public ProgressHelper<V, N>
{
public:
	// get the progress of a single node, return INF for special nodes/values
	virtual double progress(const Node<V, N>& n) = 0;
};

// -------- helper class --------

// get the progress of a single node, return INF for special nodes/values
template <typename V, typename N, bool HAS_INF = true>
struct ProgressHelperBase
{
	static double helper_progress_value(const Node<V, N>& n){
		return static_cast<double>(n.v);
	}
	static double helper_progress_vsquare(const Node<V, N>& n){
		return static_cast<double>(n.v*n.v);
	}
};
template <typename V, typename N>
struct ProgressHelperBase<V, N, false>
{
	static constexpr double MAX = std::numeric_limits<V>::max();
	static constexpr double INF = std::numeric_limits<double>::infinity();

	static double helper_progress_value(const Node<V, N>& n){
		return static_cast<double>(n.v == MAX ? INF : n.v);
	}
	static double helper_progress_vsquare(const Node<V, N>& n){
		return static_cast<double>(n.v == MAX ? INF : n.v*n.v);
	}
};
template <typename V, typename N>
struct ProgressHelper
	: public ProgressHelperBase<V, N, std::numeric_limits<V>::has_infinity>
{};

// -------- common progressor examples --------

template <typename V, typename N>
class ProgressorValue
	: public Progressor<V, N>
{
public:
	virtual double progress(const Node<V, N>& n){
		return ProgressHelper<V, N>::helper_progress_value(n);
	}
};

template <typename V, typename N>
class ProgressorSquare
	: public Progressor<V, N>
{
public:
	virtual double progress(const Node<V, N>& n){
		return ProgressHelper<V, N>::helper_progress_vsquare(n);
	}
};
