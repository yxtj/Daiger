#include "Terminator.h"
#include <cmath>
#include <stdexcept>
//#include <algorithm>

using namespace std;

// -------- TerminatorBase --------

void TerminatorBase::prepare_global_checker(const size_t n_worker){
	curr.resize(n_worker);
	sum_gp = 0.0;
	sum_gc = 0;
}
double TerminatorBase::update_report(const size_t wid, const std::pair<double, size_t>& report){
	sum_gp += report.first - curr[wid].first;
	sum_gc += report.second - curr[wid].second;
	curr[wid] = report;
}
double TerminatorBase::helper_global_progress_sum(){
	return sum_gp;
}
double TerminatorBase::helper_global_progress_sqrt(){
	return sqrt(sum_gp);
}
bool TerminatorBase::helper_no_change(const std::vector<std::pair<double, size_t>>& reports){
	for(const auto& r : reports){
		if(r.second != 0)
			return false;
	}
	return true;
}
