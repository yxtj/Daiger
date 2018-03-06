#pragma once
#include <utility>
#include <unordered_map>

struct WorkerState {
	int nid; // net id
	int wid; // worker id (local id)

	double last_comm_time;
	double last_report_time;

	//double checkpoint_time;
	//bool checkpointing;

};

// worker container for master
struct WorkerMonitor {
	void register_worker(const int nid, const int wid);

	// return <whether-a-worker, worker-id>
	std::pair<bool, int> nidtrans(const int nid) const;
	// return worker-id, assume nid is a worker's network-id
	int nid2wid(const int nid) const;

	void update_time(const int nid, const double time);
	void update_report_time(const int nid, const double time);

//private:
	std::unordered_map<int, WorkerState> cont;
};

// worker container for workers
struct WorkerIDMapper {
	void register_worker(const int nid, const int wid);
	std::pair<bool, int> nidtrans(const int nid) const;
	int nid2wid(const int nid) const;

private:
	std::unordered_map<int, int> cont;
};
