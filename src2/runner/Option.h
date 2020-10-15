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
	std::vector<std::string> partitioner_args;
	std::vector<std::string> scheduler_args;
	std::vector<std::string> terminator_args;
	std::vector<std::string> prioritizer_args;

	//float schedule_portion;
	//bool priority_degree; // use degree of out-neighbors
	//bool priority_diff; // use <u>-<v> instead of <u>
	std::string path_root;

	bool do_incremental; // when path_delta and path_value are given
	bool do_output; // when path_result is given

	float timeout; // time threshold for determining error

public:
	Option();
	~Option();

	bool parseInput(int argc, char *argv[]);
private:
	std::string& sortUpPath(std::string& path);
	double sortUpInterval(double& interval, const double min, const double max);
	std::string setWithRootPath(const std::string& relPath, const std::string& defaultPath="");
};

