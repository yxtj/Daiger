#pragma once
#include <string>
#include <vector>

class Option
{
	struct implDesc;
	implDesc* pimpl;
public:
	bool show;

	std::string path_graph;
	std::string path_delta;
	std::string path_value;
	std::string path_result;

	std::string prefix_graph;
	std::string prefix_delta;
	std::string prefix_value;
	std::string prefix_result;

	std::vector<std::string> application_parameter;

	bool balence_load; // used to support data loading from arbitrary number of input files
	size_t nPart; // optional, used to check whether a correct number of instance is started
	size_t nNode; // optional, used to preactively allocate space

	bool async;
	bool do_incremental; // when path_delta and path_value are given
	bool do_output; // when path_result is given

	float apply_interval;
	float send_interval;
	int send_batch_size;

	float schedule_portion;
	//bool priority_degree; // use degree of out-neighbors
	//bool priority_diff; // use <u>-<v> instead of <u>

public:
	Option();
	~Option();

	bool parseInput(int argc, char *argv[]);
private:
	std::string& sortUpPath(std::string& path);
	
	bool checkIncremental();
};
