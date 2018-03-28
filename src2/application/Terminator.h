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
	// get the current global progress value
	virtual double global_progress() { return helper_global_progress_sum(); }

protected:
	double helper_global_progress_sum();
	double helper_global_progress_sqrt();
	static bool helper_no_change(const std::vector<ProgressReport>& reports);

	std::vector<ProgressReport> curr;
	double sum_gp; // sum global progress
	size_t sum_gi; // sum global number of infinity
	size_t sum_gc; // sum global number of changes
};

template <typename V, typename N>
class TerminatorTypedInterface {
public:
	// on workers:
	// get the progress of a single node, return INF for special nodes/values
	virtual double progress(const Node<V, N>& n){ return helper_progress_value(n); };

protected:
	static double helper_progress_value(const Node<V, N>& n){
		return static_cast<double>(n.v);
	}
	static double helper_progress_vsquare(const Node<V, N>& n){
		return static_cast<double>(n.v*n.v);
	}
};

template <typename V, typename N>
class Terminator
	: virtual public TerminatorBase
{
public:
	// on workers:
	// get the progress of a single node, return INF for special nodes/values
	virtual double progress(const Node<V, N>& n){ return helper_progress_value(n); };

protected:
	static double helper_progress_value(const Node<V, N>& n){
		return static_cast<double>(n.v);
	}
	static double helper_progress_vsquare(const Node<V, N>& n){
		return static_cast<double>(n.v*n.v);
	}
};

// -------- an example which stops when no one changes --------

class TerminatorStopBase
	: virtual public TerminatorBase
{
public:
	virtual void prepare_global_checker(const size_t n_worker);
	virtual void update_report(const size_t wid, const ProgressReport& report);
	virtual bool check_term();
	
private:
	std::vector<size_t> last;
	size_t sum_gc_last;
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
