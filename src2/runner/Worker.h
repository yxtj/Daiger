#pragma once
#include "Runner.h"
#include "network/RPCInfo.h"
#include <string>

class NetworkThread;

class Worker : public Runner {
public:
    Worker() = default;
    Worker(const AppBase& app, Option& opt);
	
	virtual void start();
	virtual void finish();

protected:
	virtual void registerWorker();
	virtual void terminateWorker();

	virtual void procedureLoadGraph();
	virtual void procedureLoadValue();
	virtual void procedureLoadDelta();
	virtual void procedureUpdate();
	virtual void procedureOutput();

// handlers
private:
	//using callback_t = void (Worker::*)(const std::string&, const RPCInfo&);
	//typedef void (Worker::*callback_t)(const string&, const RPCInfo&);
	using typename Runner::callback_t;
	callback_t localBinder(void (Worker::*fp)(const std::string&, const RPCInfo&));
	virtual void registerHandlers();

private:

};
