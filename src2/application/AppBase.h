#pragma once
#include "ArgumentSeparator.h"
#include "Operation.h"
#include "IOHandler.h"
#include "Terminator.h"
#include "Partitioner.h"
#include "Scheduler.h"
#include "holder/GlobalHolderBase.h"
#include <string>
#include <vector>

struct AppBase {
	AppBase(); // set every pointer to null

	// application-related (also type-related)
	OperationBase* opt;
	TerminatorBase* tmt;
	IOHandlerBase* ioh;

	// application-independent
	PartitionerBase* ptn;
	SchedulerBase* scd;

	GlobalHolderBase* gh; // graph holder

	bool needOutNeighbor;
	bool needInNeighbor;
	
	bool check() const;
	void clear();
};

AppBase makeApplication(const std::string& app_name, const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_partitioner, const std::vector<std::string>& arg_scheduler,
	const size_t nInstance);
