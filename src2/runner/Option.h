#pragma once
#include "common/ConfData.h"
#include <string>
#include <vector>

class Option
{
	struct implDesc;
	implDesc* pimpl;
public:
	bool show;

	ConfData conf;

	std::string app_name;
	std::vector<std::string> app_args;
	std::vector<std::string> sharder_args;
	std::vector<std::string> scheduler_args;

	//float schedule_portion;
	//bool priority_degree; // use degree of out-neighbors
	//bool priority_diff; // use <u>-<v> instead of <u>

	size_t nPart; // optional, used to check whether a correct number of instance is started
	size_t nNode; // optional, used to preactively allocate space

	bool do_incremental; // when path_delta and path_value are given
	bool do_output; // when path_result is given

	float timeout; // time threshold for determining error

	//float sleep_interval;
	float apply_interval;
	float send_interval;
	float term_interval;

public:
	Option();
	~Option();

	bool parseInput(int argc, char *argv[]);
private:
	std::string& sortUpPath(std::string& path);
	float sortUpInterval(float& interval, const float min, const float max);
};

