#pragma once
#include "ProgressReport.h"
#include <vector>
#include <string>
#include <utility>
#include <limits>

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

// -------- an example which stops when no one changes --------

class TerminatorStop
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

// -------- an example of a difference-based terminator --------

class TerminatorDiffValue
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

class TerminatorDiffRatio
	: virtual public TerminatorDiffValue
{
public:
	virtual void init(const std::vector<std::string>& args);
	virtual bool check_term();

private:
	std::vector<std::pair<double, size_t>> last; // sum, n_inf
	double sum_gp_last;
	size_t sum_gi_last;
	double ratio;
	double epsilon;
	bool untouched;
};

// -------- an example of a variance-based terminator --------
// calculate an average level, and check whether the variance is smaller than a scale
// the average level is maintained with exponential average

class TerminatorVariance
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
