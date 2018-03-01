#pragma once
#include <string>
#include <vector>

class OperationBase;
class TerminatorBase;
class SharderBase;
class SchedulerBase;
class IOHanderBase;
class LocalHolderBase;
class RemoteHolderBase;

class AppBase{
public:
	virtual OperationBase* getOperation() = 0;
	virtual TerminatorBase* getTerminator() = 0;
	virtual IOHanderBase* getIOHander() = 0;

	virtual SharderBase* getShader() = 0;
	virtual SchedulerBase* getScheduler() = 0;

	virtual LocalHolderBase* getLocalHolder() = 0;
	virtual RemoteHolderBase* getRemoteHolder() = 0;
};

struct AppKernel{
	OperationBase* pop;
	TerminatorBase* ptm;
	IOHanderBase* pio;

	SharderBase* psd;
	SchedulerBase* psc;
};

AppBase* makeApplication(const std::vector<std::string>& arg_app, 
	const std::vector<std::string>& arg_sharder, const std::vector<std::string>& arg_scheduler);
