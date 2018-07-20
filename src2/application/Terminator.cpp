#include "Terminator.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

using namespace std;

// -------- TerminatorBase --------

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

void TerminatorStopBase::prepare_global_checker(const size_t n_worker){
	TerminatorBase::prepare_global_checker(n_worker);
	last.resize(n_worker);
	sum_last = sum;
	untouched = true;
}

void TerminatorStopBase::update_report(const size_t wid, const ProgressReport& report){
	untouched = false;
	//sum_gc_last += TerminatorBase::curr[wid].n_change - last[wid];
	last[wid] = TerminatorBase::curr[wid];
	sum_last = sum;
	TerminatorBase::update_report(wid, report);
}

bool TerminatorStopBase::check_term(){
	return !untouched && sum.n_change == 0 && sum_last.n_change == 0;
}

std::pair<double, size_t> TerminatorStopBase::difference(){
	return make_pair(sum.sum - sum_last.sum,
		sum.n_inf - sum_last.n_inf);
}

// -------- TerminatorDiff --------

void TerminatorDiffBase::init(const std::vector<std::string>& args){
	try{
		epsilon = stod(args[0]);
	} catch (exception& e){
		throw invalid_argument("Unable to get <epsilon> for TerminatorDiff.");
	}
}

void TerminatorDiffBase::prepare_global_checker(const size_t n_worker){
	TerminatorBase::prepare_global_checker(n_worker);
	last.resize(n_worker);
	sum_gp_last=0.0;
	sum_gi_last=0;
	untouched = true;
}

void TerminatorDiffBase::update_report(const size_t wid, const ProgressReport& report){
	untouched = false;
	sum_gp_last += TerminatorBase::curr[wid].sum - last[wid].first;
	sum_gi_last += TerminatorBase::curr[wid].n_inf - last[wid].second;
	last[wid].first = TerminatorBase::curr[wid].sum;
	last[wid].second = TerminatorBase::curr[wid].n_inf;
	TerminatorBase::update_report(wid, report);
}

bool TerminatorDiffBase::check_term(){
	return !untouched && sum_gi_last == sum.n_inf
		&& fabs(sum_gp_last - sum.sum) < epsilon;
}

std::pair<double, size_t> TerminatorDiffBase::difference(){
	return make_pair(sum.sum - sum_gp_last,
		sum.n_inf - sum_gi_last);
}
