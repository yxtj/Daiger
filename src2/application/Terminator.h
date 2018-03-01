#pragma once
#include "common/Node.h"
#include <vector>
#include <string>
#include <utility>

class TerminatorBase {
public:
	virtual void init(const std::vector<std::string>& arg){};

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

	std::vector<std::pair<double, size_t>> curr;
	double sum_gp;
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
	virtual void init(const std::vector<std::string>& arg);
	virtual void prepare_global_checker(const size_t n_worker);
	virtual double update_report(const size_t wid, const std::pair<double, size_t>& report);
	virtual bool check_term();
	
private:
	std::vector<double> last;
	double sum_gp_last;
	double epsilon;
};

template <typename V, typename N>
void TerminatorDiff<V, N>::init(const std::vector<std::string>& arg){
	try{
		epsilon = stod(arg[0]);
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
	sum_gp += report.first - curr[wid].first;
	curr[wid] = report;
}
template <typename V, typename N>
bool TerminatorDiff<V, N>::check_term(){
	return TerminatorBase::check_term() && abs(sum_gp_last - sum_gp) < epsilon;
}
