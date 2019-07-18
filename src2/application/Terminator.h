#pragma once
#include "common/Node.h"
#include <vector>
#include <string>
#include <utility>
#include <limits>

struct ProgressReport{
	double sum; // summation of the non-infinity progress value
	size_t n_inf; // # of infinity progress values
	size_t n_change; // # of changed nodes
};

class TerminatorBase {
public:
	virtual ~TerminatorBase() = default;
	virtual void init(const std::vector<std::string>& args){};

	static constexpr double INF = std::numeric_limits<double>::infinity();

	// on workers:
	//// get the progress of a single node (template function cannot be virtual)
	//template <typename V, typename N>
	//virtual double progress(const Node<V, N>& n){ return helper_progress_value(n); }; 

	// on master:
	// initialize the curr variable, only needed on master
	virtual void prepare_global_checker(const size_t n_worker);
	// report format: (local progress, # nodes whose v changed after last report)
	virtual void update_report(const size_t wid, const ProgressReport& report);
	// check whether to terminate via progress reports, default: no-one changed;
	virtual bool check_term() = 0;
	// get the progress state & improvement over the last report
	std::pair<double, size_t> state();
	virtual std::pair<double, size_t> difference() = 0;
	// get the current global progress value
	virtual double global_progress() { return helper_global_progress_sum(); }

protected:
	double helper_global_progress_sum();
	double helper_global_progress_sqrt();
	static bool helper_no_change(const std::vector<ProgressReport>& reports);

	std::vector<ProgressReport> curr;
	ProgressReport sum; // summary of all elements in curr (by accumulating)
};

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

template <typename V, typename N>
class Terminator
	: virtual public TerminatorBase, public ProgressHelper<V, N>
{
public:
	// on workers:
	// get the progress of a single node, return INF for special nodes/values
	virtual double progress(const Node<V, N>& n){
		return ProgressHelper<V, N>::helper_progress_value(n);
	};
};

// -------- an example which stops when no one changes --------

class TerminatorStopBase
	: virtual public TerminatorBase
{
public:
	virtual void prepare_global_checker(const size_t n_worker);
	virtual void update_report(const size_t wid, const ProgressReport& report);
	virtual bool check_term();
	virtual std::pair<double, size_t> difference();
	
private:
	std::vector<ProgressReport> last;
	ProgressReport sum_last;
	bool untouched;
};

template <typename V, typename N>
class TerminatorStop
	: public TerminatorStopBase, public Terminator<V, N>
{};

// -------- an example of a difference-based terminator --------

class TerminatorDiffBase
	: virtual public TerminatorBase
{
public:
	virtual void init(const std::vector<std::string>& args);
	virtual void prepare_global_checker(const size_t n_worker);
	virtual void update_report(const size_t wid, const ProgressReport& report);
	virtual bool check_term();
	virtual std::pair<double, size_t> difference();
	
private:
	std::vector<std::pair<double, size_t>> last; // sum, n_inf
	double sum_gp_last;
	size_t sum_gi_last;
	double epsilon;
	bool untouched;
};

template <typename V, typename N>
class TerminatorDiff
	: public TerminatorDiffBase, public Terminator<V, N>
{};

// -------- an example of a variance-based terminator --------
// calculate an average level, and check whether the variance is smaller than a scale
// the average level is maintained with exponential average

class TerminatorVarianceBase
	: virtual public TerminatorBase
{
public:
	virtual void init(const std::vector<std::string>& args);
	virtual void prepare_global_checker(const size_t n_worker);
	virtual void update_report(const size_t wid, const ProgressReport& report);
	virtual bool check_term();
	virtual std::pair<double, size_t> difference();

private:
	std::vector<ProgressReport> last;
	ProgressReport sum_last;
	double average;
	double decay; // = var_portion/n_worker
	double var_portion; // the portion threshold
	bool untouched;
};

template <typename V, typename N>
class TerminatorVariance
	: public TerminatorVarianceBase, public Terminator<V, N>
{};
