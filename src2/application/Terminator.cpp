#include "Terminator.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

using namespace std;

// -------- TerminatorBase --------

void TerminatorBase::prepare_global_checker(const size_t n_worker){
	curr.resize(n_worker);
	sum_gp = 0.0;
	sum_gi = 0;
	sum_gc = 0;
}
void TerminatorBase::update_report(const size_t wid, const ProgressReport& report){
	sum_gp += report.sum - curr[wid].sum;
	sum_gi += report.n_inf - curr[wid].n_inf;
	sum_gc += report.n_change - curr[wid].n_change;
	curr[wid] = report;
}
double TerminatorBase::helper_global_progress_sum(){
	return sum_gp;
}
double TerminatorBase::helper_global_progress_sqrt(){
	return sqrt(sum_gp);
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
	sum_gc_last=0;
	untouched = true;
}

void TerminatorStopBase::update_report(const size_t wid, const ProgressReport& report){
	untouched = false;
	sum_gc_last += TerminatorBase::curr[wid].n_change - last[wid];
	last[wid] = TerminatorBase::curr[wid].n_change;
	TerminatorBase::update_report(wid, report);
}

bool TerminatorStopBase::check_term(){
	return !untouched && TerminatorBase::sum_gc == 0 && sum_gc_last == 0;
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
	return !untouched && TerminatorBase::sum_gc == 0
		&& sum_gi_last == TerminatorBase::sum_gi
		&& fabs(sum_gp_last - TerminatorBase::sum_gp) < epsilon;
}
