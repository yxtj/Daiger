#pragma once
#include <string>

struct ConfData {
	size_t nPart;
	size_t nNode; // optional, used to preactively allocate space

	bool balance_load; // used to support data loading from arbitrary number of input files
	bool aggregate_message; // used to save network bandwidth

	std::string path_graph;
	std::string path_delta;
	std::string path_value;
	std::string path_result;

	std::string prefix_graph;
	std::string prefix_delta;
	std::string prefix_value;
	std::string prefix_result;

	bool async;
	bool cache_free;
	bool sort_result; // whether to sort the output by node id

	double apply_interval; // applying interval
	double apply_max_portion; // max number of node processed
	double apply_min_portion; // min number of node processed before apply

	double send_interval; // sending interval
	int send_max_size; // the maximum # of nodes in each message
	int send_min_size; // before reaching send_max_interval, the minimum # of nodes in each message

	double progress_interval;
	double termination_max_time;
};
