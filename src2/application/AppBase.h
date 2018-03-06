#pragma once
#include "ArgumentSeparator.h"
#include "Operation.h"
#include "IOHandler.h"
#include "Terminator.h"
#include "Sharder.h"
#include "Scheduler.h"
#include <string>
#include <vector>

/*
class OperationBase;
class TerminatorBase;
class SharderBase;
class SchedulerBase;
class IOHandlerBase;

class LocalHolderBase;
class RemoteHolderBase;
class AppBase{
public:
	// application-related (also type-related)
	virtual OperationBase* getOperation() = 0;
	virtual TerminatorBase* getTerminator() = 0;
	virtual IOHanderBase* getIOHander() = 0;

	// application-independent
	virtual SharderBase* getShader() = 0;
	virtual SchedulerBase* getScheduler() = 0;

	virtual LocalHolderBase* getLocalHolder() = 0;
	virtual RemoteHolderBase* getRemoteHolder() = 0;
};
*/

struct AppBase {
	// application-related (also type-related)
	OperationBase* opt;
	TerminatorBase* tmt;
	IOHandlerBase* ioh;

	// application-independent
	SharderBase* shd;
	SchedulerBase* scd;
};

AppBase makeApplication(const std::string& app_name, const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_sharder, const std::vector<std::string>& arg_scheduler);
