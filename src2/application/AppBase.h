#pragma once
#include "ArgumentSeparator.h"
#include "Operation.h"
#include "IOHandler.h"
#include "Progressor.h"
#include "Partitioner.h"
#include "Scheduler.h"
#include "Terminator.h"
#include "holder/GlobalHolderBase.h"
#include <string>
#include <vector>

struct AppBase {
	AppBase(); // set every pointer to null

	// application-related (also type-related)
	OperationBase* opt;
	ProgressorBase* prg;
	IOHandlerBase* ioh;

	// application-independent
	PartitionerBase* ptn;
	SchedulerBase* scd;
	TerminatorBase* tmt;

	GlobalHolderBase* gh; // graph holder

	bool needOutNeighbor;
	bool needInNeighbor;
	
	bool check() const;
	void clear();
};

AppBase makeApplication(const std::string& app_name, const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_partitioner, const std::vector<std::string>& arg_scheduler,
	const std::vector<std::string>& arg_terminator, const size_t nInstance);
