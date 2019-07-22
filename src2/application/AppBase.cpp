#include "AppBase.h"
#include "factory/AppKernelFactory.h"
//#include "factory/ArgumentSeparatorFactory.h"
//#include "factory/OperationFactory.h"
//#include "factory/IOHandlerFactory.h"

#include "factory/PartitionerFactory.h"
#include "factory/SchedulerFactory.h"
#include "factory/TerminatorFactory.h"

using namespace std;

AppBase::AppBase()
	: opt(nullptr), prg(nullptr), ioh(nullptr),
	ptn(nullptr), scd(nullptr), gh(nullptr)
{}

bool AppBase::check() const {
	return opt!=nullptr && prg!=nullptr && ioh!=nullptr
		&& ptn!=nullptr && scd!=nullptr && tmt != nullptr
		&& gh!=nullptr;
}

void AppBase::clear() {
	delete opt;
	opt = nullptr;
	delete prg;
	prg = nullptr;
	delete ioh;
	ioh = nullptr;
	delete ptn;
	ptn = nullptr;
	delete scd;
	scd = nullptr;
	delete gh;
	gh = nullptr;
}

AppBase makeApplication(const std::string& app_name, const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_partitioner, const std::vector<std::string>& arg_scheduler,
	const std::vector<std::string>& arg_terminator, const size_t nInstance)
{
	AppBase app;
	AppKernel* apk = AppKernelFactory::generate(app_name);
	ArgumentSeparator* sep = apk->generateSeparator();
	AppArguments aa = sep->separate(arg_app);
	delete sep;
	app.opt = apk->generateOperation();
	app.opt->init(aa.operation_arg, nInstance);
	app.ioh = apk->generateIOHandler();
	app.ioh->init(aa.iohandler_arg);
	app.prg = apk->generateProgressor();
	app.prg->init(aa.progressor_arg);
	app.gh = apk->generateGraph();
	// gh should be initialized later in Worker::registerWorkers()
	app.needInNeighbor = apk->needInNeighbor();
	app.needOutNeighbor = apk->needOutNeighbor();
	delete apk;

	app.ptn = PartitionerFactory::generate(arg_partitioner[0]);
	app.ptn->init(arg_partitioner);
	app.scd = SchedulerFactory::generate(arg_scheduler[0]);
	app.scd->init(arg_scheduler);
	app.tmt = TerminatorFactory::generate(arg_terminator[0]);
	app.tmt->init(arg_terminator);

	return app;
}
