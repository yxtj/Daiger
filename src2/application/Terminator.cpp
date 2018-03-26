#include "Terminator.h"
#include <cmath>
#include <stdexcept>
//#include <algorithm>

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
