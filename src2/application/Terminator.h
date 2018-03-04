#pragma once
#include "common/Node.h"
#include <vector>
#include <string>
#include <utility>

class TerminatorBase {
public:
	virtual void init(const std::vector<std::string>& args){};

	// on workers:
	//// get the progress of a single node (template function cannot be virtual)
	//template <typename V, typename N>
	//virtual double progress(const Node<V, N>& n){ return helper_progress_value(n); }; 

	// on master:
	// initialize the curr variable, only needed on master
	virtual void prepare_global_checker(const size_t n_worker);
	// report format: (local progress, # ndoes whose v changed after last report)
	virtual double update_report(const size_t wid, const std::pair<double, size_t>& report);
	// check whetherto terminate via progress reports, default: no-one changed;
	virtual bool check_term() = 0;
	// get the current global progress value
	virtual double global_progress() { return helper_global_progress_sum(); }

protected:
	double helper_global_progress_sum();
	double helper_global_progress_sqrt();
	static bool helper_no_change(const std::vector<std::pair<double, size_t>>& reports);

	std::vector<std::pair<double, size_t>> curr;
	double sum_gp; // sum global rogress
	size_t sum_gc; // sum global number of changes
};

template <typename V, typename N>
class TerminatorBaseT
	: public TerminatorBase
{
public:
	// on workers:
	// get the progress of a single node
	virtual double progress(const Node<V, N>& n){ return helper_progress_value(n); };

protected:
	virtual double helper_progress_value(const Node<V, N>& n){
		return static_cast<double>(n.v);
	}
	virtual double helper_progress_vsquare(const Node<V, N>& n){
		return static_cast<double>(n.v*n.v);
	}
};

// -------- an example of a difference-based terminator --------

template <typename V, typename N>
class TerminatorDiff
	: public TerminatorBaseT<V, N>
{
public:
	virtual void init(const std::vector<std::string>& args);
	virtual void prepare_global_checker(const size_t n_worker);
	virtual double update_report(const size_t wid, const std::pair<double, size_t>& report);
	virtual bool check_term();
	
private:
	std::vector<double> last;
	double sum_gp_last;
	double epsilon;
};

template <typename V, typename N>
void TerminatorDiff<V, N>::init(const std::vector<std::string>& args){
	try{
		epsilon = stod(args[0]);
	} catch (exception& e){
		throw invalid_argument("Unable to get <epsilon> for TerminatorDiff.");
	}
}
template <typename V, typename N>
void TerminatorDiff<V, N>::prepare_global_checker(const size_t n_worker){
	curr.resize(n_worker);
	last.resize(n_worker);
	sum_gp=0.0;
	sum_gp_last=0.0;
}
template <typename V, typename N>
double TerminatorDiff<V, N>::update_report(const size_t wid, const std::pair<double, size_t>& report){
	sum_gp_last += curr[wid].first - last[wid];
	last[wid] = curr[wid].first;
	TerminatorBase::update_report(wid, report);
}
template <typename V, typename N>
bool TerminatorDiff<V, N>::check_term(){
	return TerminatorBase::helper_no_change(curr) && abs(sum_gp_last - sum_gp) < epsilon;
}

// -------- an example which stops when no one changes --------

template <typename V, typename N>
class TerminatorStop
	: public TerminatorBaseT<V, N>
{
public:
	virtual void prepare_global_checker(const size_t n_worker);
	virtual double update_report(const size_t wid, const std::pair<double, size_t>& report);
	virtual bool check_term();
	
private:
	std::vector<size_t> last;
	size_t sum_gc_last;
};

template <typename V, typename N>
void TerminatorStop<V, N>::prepare_global_checker(const size_t n_worker){
	curr.resize(n_worker);
	last.resize(n_worker);
	sum_gc=0;
	sum_gc_last=std::numeric_limits<size_t>::max();
}
template <typename V, typename N>
double TerminatorStop<V, N>::update_report(const size_t wid, const std::pair<double, size_t>& report){
	sum_gc_last += curr[wid].second - last[wid];
	last[wid] = curr[wid].second;
	TerminatorBase::update_report(wid, report);
}
template <typename V, typename N>
bool TerminatorStop<V, N>::check_term(){
	return sum_gc == 0 && sum_gc_last == 0;
}
