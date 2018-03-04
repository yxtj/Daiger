#include "AppBase.h"
#include "factory/ArgumentSeparatorFactory.h"
#include "factory/OperationFactory.h"
#include "factory/IOHandlerFactory.h"
#include "factory/TerminatorFactory.h"

#include "factory/SharderFactory.h"
#include "factory/SchedulerFactory.h"

using namespace std;

AppBase makeApplication(const std::string& app_name, const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_sharder, const std::vector<std::string>& arg_scheduler)
{
	AppBase app;
	ArgumentSeparator* sep = ArgumentSeparatorFactory::generate(app_name);
	AppArguments aa = sep->separate(arg_app);
	delete sep;
	app.opt = OperationFactory::generate(app_name);
	app.opt->init(aa.operation_arg);
	app.ioh = IOHandlerFactory::generate(app_name);
	app.ioh->init(aa.iohandler_arg);
	app.tmt = TerminatorFactory::generate(app_name);
	app.tmt->init(aa.terminator_arg);

	app.shd = SharderFactory::generate(arg_sharder[0]);
	app.shd->init(arg_sharder);
	app.scd = SharderFactory::generate(arg_scheduler[0]);
	app.scd->init(arg_scheduler);
	return app;
}
