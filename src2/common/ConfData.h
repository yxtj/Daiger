#pragma once
#include <string>

struct ConfData {
	size_t nPart;
	size_t nNode; // optional, used to preactively allocate space

	bool balance_load; // used to support data loading from arbitrary number of input files

	std::string path_graph;
	std::string path_delta;
	std::string path_value;
	std::string path_result;

	std::string prefix_graph;
	std::string prefix_delta;
	std::string prefix_value;
	std::string prefix_result;

	int send_batch_size;

	bool async;
	bool cache_free;
	bool sort_result; // whether to sort the output by node id
};
