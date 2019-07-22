#include "Terminator.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

using namespace std;

// -------- Terminator Base --------

void TerminatorBase::prepare_global_checker(const size_t n_worker){
	curr.resize(n_worker);
	sum.sum = 0.0;
	sum.n_inf = 0;
	sum.n_change = 0;
}

void TerminatorBase::update_report(const size_t wid, const ProgressReport& report){
	auto& item = curr[wid];
	sum.sum += report.sum - item.sum;
	sum.n_inf += report.n_inf - item.n_inf;
	sum.n_change += report.n_change - item.n_change;
	item = report;
}

std::pair<double, size_t> TerminatorBase::state(){
	return make_pair(sum.sum, sum.n_inf);
}

double TerminatorBase::helper_global_progress_sum(){
	return sum.sum;
}
double TerminatorBase::helper_global_progress_sqrt(){
	return sqrt(sum.sum);
}
bool TerminatorBase::helper_no_change(const std::vector<ProgressReport>& reports){
	for(const auto& r : reports){
		if(r.n_change != 0)
			return false;
	}
	return true;
}

// -------- TerminatorStop --------

void TerminatorStop::prepare_global_checker(const size_t n_worker){
	TerminatorBase::prepare_global_checker(n_worker);
	last.resize(n_worker);
	sum_last = sum;
	untouched = true;
}

void TerminatorStop::update_report(const size_t wid, const ProgressReport& report){
	untouched = false;
	//sum_gc_last += TerminatorBase::curr[wid].n_change - last[wid];
	last[wid] = TerminatorBase::curr[wid];
	sum_last = sum;
	TerminatorBase::update_report(wid, report);
}

bool TerminatorStop::check_term(){
	return !untouched && sum.n_change == 0 && sum_last.n_change == 0;
}

std::pair<double, size_t> TerminatorStop::difference(){
	return make_pair(sum.sum - sum_last.sum,
		sum.n_inf - sum_last.n_inf);
}

// -------- TerminatorDiffValue --------

void TerminatorDiffValue::init(const std::vector<std::string>& args){
	try{
		epsilon = stod(args[0]);
	} catch (exception& e){
		throw invalid_argument("Unable to get <epsilon> for TerminatorDiffValue.");
	}
}

void TerminatorDiffValue::prepare_global_checker(const size_t n_worker){
	TerminatorBase::prepare_global_checker(n_worker);
	last.resize(n_worker);
	sum_gp_last=0.0;
	sum_gi_last=0;
	untouched = true;
}

void TerminatorDiffValue::update_report(const size_t wid, const ProgressReport& report){
	untouched = false;
	sum_gp_last += TerminatorBase::curr[wid].sum - last[wid].first;
	sum_gi_last += TerminatorBase::curr[wid].n_inf - last[wid].second;
	last[wid].first = TerminatorBase::curr[wid].sum;
	last[wid].second = TerminatorBase::curr[wid].n_inf;
	TerminatorBase::update_report(wid, report);
}

bool TerminatorDiffValue::check_term(){
	return !untouched && sum_gi_last == sum.n_inf
		&& fabs(sum_gp_last - sum.sum) < epsilon;
}

std::pair<double, size_t> TerminatorDiffValue::difference(){
	return make_pair(sum.sum - sum_gp_last,
		sum.n_inf - sum_gi_last);
}

// -------- TerminatorDiffRatio--------

void TerminatorDiffRatio::init(const std::vector<std::string>& args){
	try{
		ratio = stod(args[0]);
	} catch(exception& e){
		throw invalid_argument("Unable to get <epsilon> for TerminatorDiffRatio.");
	}
}

bool TerminatorDiffRatio::check_term(){
	return !untouched && sum_gi_last == sum.n_inf
		&& fabs(sum_gp_last - sum.sum)/sum_gp_last < ratio;
}


// -------- TerminatorVariance --------

void TerminatorVariance::init(const std::vector<std::string>& args)
{
	try{
		var_portion = stod(args[0]);
		decay = var_portion; // modified when n_worker is given
	} catch(exception& e){
		throw invalid_argument("Unable to get parameters for TerminatorVariance.");
	}
}

void TerminatorVariance::prepare_global_checker(const size_t n_worker)
{
	TerminatorBase::prepare_global_checker(n_worker);
	last.resize(n_worker);
	average = 0.0;
	decay = var_portion / n_worker;
	untouched = true;
}

void TerminatorVariance::update_report(const size_t wid, const ProgressReport & report)
{
	untouched = false;
	double new_average = average - TerminatorBase::curr[wid].sum + report.sum;
	average = average * (1 - decay) + new_average * decay;
	last[wid] = TerminatorBase::curr[wid];
	sum_last = sum;
	TerminatorBase::update_report(wid, report);
}

bool TerminatorVariance::check_term()
{
	return !untouched && abs(average-sum.sum)/average < var_portion;
}

std::pair<double, size_t> TerminatorVariance::difference()
{
	return make_pair(sum.sum - sum_last.sum,
		sum.n_inf - sum_last.n_inf);
}
