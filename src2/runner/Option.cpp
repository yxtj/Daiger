#include "Option.h"
#include "util/Util.h"
#include <iostream>
#include <algorithm>
#include <boost/program_options.hpp>

using namespace std;

struct Option::implDesc {
	boost::program_options::options_description desc;
};

Option::Option()
	:pimpl(new implDesc{ boost::program_options::options_description("Options", getScreenSize().first) })
{
	// define
	using boost::program_options::value;
	pimpl->desc.add_options()
		("help", "Print help messages")
		("show_info", value<bool>(&show)->default_value(1), "Print the initializing information.")
		("load_balance", value<bool>(&conf.balance_load)->default_value(1), "Support loading from arbitrary number of files.")
		("part", value<size_t>(&nPart)->default_value(0), 
			"[integer] # of workers, used check whether a correct number of instance is started.")
		("node", value<size_t>(&nNode)->default_value(-1), 
			"[integer] # of nodes, used for preactively allocate space.")
		("path_graph", value<string>(&conf.path_graph), "Path of the input graph files.")
		("prefix_graph", value<string>(&conf.prefix_graph)->default_value(string("part-")), "Prefix of the input graph files.")
		("path_delta", value<string>(&conf.path_delta), "Path of the delta graph files. If not given, do non-incremental computation.")
		("prefix_delta", value<string>(&conf.prefix_delta)->default_value(string("delta-")), "Prefix of the delta graph files.")
		("path_value", value<string>(&conf.path_value), "Path of the initial value files. If not given, do non-incremental computation.")
		("prefix_value", value<string>(&conf.prefix_value)->default_value(string("value-")), "Prefix of the initial value files.")
		("path_result", value<string>(&conf.path_result), "Path of the output value files. If not given, do NOT output result.")
		("prefix_result", value<string>(&conf.prefix_result)->default_value(string("res-")), "Prefix of the output value files.")
		("app", value<string>(&app_name), "The name of the application to run.")
		("app_args", value<vector<string>>(&app_args)->multitoken()->default_value({}, ""), "Application parameters.")
		("sharder", value<vector<string>>(&sharder_args)->multitoken()->default_value({"mod"}, ""),
			"Sharder strategy name and parameters. Supports: mod.")
		("scheduler", value<vector<string>>(&scheduler_args)->multitoken()->default_value({"priority", ""}, ""),
			"Scheduler name and parameters. Supports: rr, priority, fifo.")
		("async", value<bool>(&async)->default_value(1), "Whether to perform asynchronous computation.")
		("timeout", value<float>(&timeout)->default_value(1.0f), "[float] time threshold (second) for determining error.")
		//("schedule_portion", value<float>(&schedule_portion)->default_value(1.0f), "[float] the portion of nodes used in each run.")
		//("priority_degree", value<bool>(&priority_degree)->default_value(0), "Whether to use degree information in priority function.")
		("apply_interval", value<float>(&apply_interval)->default_value(0.5f), "[float] the maximum interval (second) of performing apply.")
		("send_interval", value<float>(&send_interval)->default_value(0.5f), "[float] the maximum interval (second) of performing send.")
		("term_interval", value<float>(&term_interval)->default_value(0.5f),
			"[float] the minimum interval (second) of reporting progress and do termination check.")
		("send_batch_size", value<int>(&conf.send_batch_size)->default_value(1024),
			"[integer] the maximum size (# of target nodes) of each sending message.")
		;
}

Option::~Option() {
	delete pimpl;
}

bool Option::parseInput(int argc, char* argv[]) {
	//parse
	bool flag_help = false;
	boost::program_options::variables_map var_map;
	try {
		boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, pimpl->desc), var_map);
		boost::program_options::notify(var_map);

		if(var_map.count("help")) {
			flag_help = true;
		}
	} catch(std::exception& e) {
		cerr << "error: " << e.what() << "\n";
		flag_help = true;
	} catch(...) {
		cerr << "Exception of unknown type!\n";
		flag_help = true;
	}

	do {
		if(conf.path_graph.empty()) {
			cerr << "Graph path is not given" << endl;
			flag_help = true;
			break;
		}
		sortUpPath(conf.path_graph);
		sortUpPath(conf.path_delta);
		sortUpPath(conf.path_value);
		sortUpPath(conf.path_result);

		if(conf.path_delta.empty() || conf.path_value.empty())
			do_incremental=false;
		
		if(conf.path_result.empty())
			do_output=false;

		sortUpInterval(apply_interval, 0.001, 10.0);
		sortUpInterval(send_interval, 0.001, 10.0);
		sortUpInterval(term_interval, 2*apply_interval, 600.0);

		break;
	} while(false); // technique for condition checking

	if(true == flag_help) {
		cerr << pimpl->desc << endl;
		return false;
	}
	return true;
}

std::string& Option::sortUpPath(std::string & path)
{
	if(!path.empty() && path.back() != '/')
		path.push_back('/');
	return path;
}

float Option::sortUpInterval(float& interval, const float min, const float max){
	interval = std::min(min, interval);
	interval = std::max(max, interval);
	return interval;
}
