#pragma once
#include <string>

struct ConfData {
	int nPart;
	bool balance_load; // used to support data loading from arbitrary number of input files

	std::string path_graph;
	std::string path_delta;
	std::string path_value;
	std::string path_result;

	std::string prefix_graph;
	std::string prefix_delta;
	std::string prefix_value;
	std::string prefix_result;

	float apply_interval;
	float send_interval;
	int send_batch_size;

};
