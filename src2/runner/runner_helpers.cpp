#include "runner_helpers.h"
#include "util/Timer.h"

using namespace std;


void WorkerMonitor::register_worker(const int nid, const int wid){
	WorkerState w;
	w.nid = nid;
	w.wid = wid;
	double now = Timer::Now();
	w.last_comm_time = now;
	w.last_report_time = now;
	cont[nid] = move(w);
}

// return <whether-a-worker, worker/master-id>
std::pair<bool, int> WorkerMonitor::nidtrans(const int nid) const {
	auto it = cont.find(nid);
	if(it == cont.end()){
		return make_pair(false, nid);
	}else{
		return make_pair(true, it->second.wid);
	}		
}

int WorkerMonitor::nid2wid(const int nid) const {
	return cont.at(nid).wid;
}

void WorkerMonitor::update_time(const int nid, const double time){
	cont[nid].last_comm_time = time;
}

void WorkerMonitor::update_report_time(const int nid, const double time){
	cont[nid].last_comm_time = time;
	cont[nid].last_report_time = time;
}

// -------- WorkerIDMapper --------

void WorkerIDMapper::register_worker(const int nid, const int wid){
	cont[nid] = wid;
}

// return <whether-a-worker, worker/master-id>
std::pair<bool, int> WorkerIDMapper::nidtrans(const int nid) const {
	auto it = cont.find(nid);
	if(it == cont.end()){
		return make_pair(false, nid);
	}else{
		return make_pair(true, it->second);
	}		
}

int WorkerIDMapper::nid2wid(const int nid) const {
	return cont.at(nid);
}
