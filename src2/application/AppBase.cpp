#include "AppBase.h"
#include "factory/AppKernelFactory.h"
//#include "factory/ArgumentSeparatorFactory.h"
//#include "factory/OperationFactory.h"
//#include "factory/IOHandlerFactory.h"
//#include "factory/TerminatorFactory.h"

#include "factory/SharderFactory.h"
#include "factory/SchedulerFactory.h"

using namespace std;

AppBase::AppBase()
	: opt(nullptr), tmt(nullptr), ioh(nullptr),
	shd(nullptr), scd(nullptr), gh(nullptr)
{}

bool AppBase::check() const {
	return opt!=nullptr && tmt!=nullptr && ioh!=nullptr
		&& shd!=nullptr && scd!=nullptr && gh!=nullptr;
}

AppBase makeApplication(const std::string& app_name, const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_sharder, const std::vector<std::string>& arg_scheduler)
{
	AppBase app;
	AppKernel* apk = AppKernelFactory::generate(app_name);
	ArgumentSeparator* sep = apk->generateSeparator();
	AppArguments aa = sep->separate(arg_app);
	delete sep;
	app.opt = apk->generateOperation();
	app.opt->init(aa.operation_arg);
	app.ioh = apk->generateIOHandler();
	app.ioh->init(aa.iohandler_arg);
	app.tmt = apk->generateTerminator();
	app.tmt->init(aa.terminator_arg);
	app.gh = apk->generateGraph();
	// gh should be initialized later in Worker::registerWorkers()
	delete apk;

	app.shd = SharderFactory::generate(arg_sharder[0]);
	app.shd->init(arg_sharder);
	// shd should be given number of worker later in main()
	app.scd = SchedulerFactory::generate(arg_scheduler[0]);
	app.scd->init(arg_scheduler);

	return app;
}
