#pragma once
#include "Runner.h"
#include "network/RPCInfo.h"
#include "runner_helpers.h"
#include "holder/GlobalHolder.h"
#include <string>
#include <thread>

class NetworkThread;

class Worker : public Runner {
public:
    Worker() = default;
    Worker(const AppBase& app, Option& opt);
	
	virtual void start();
	virtual void finish();

protected:
	virtual void registerWorker();
	virtual void shutdownWorker();
	virtual void terminateWorker();

	virtual void procedureLoadGraph();
	virtual void procedureLoadValue();
	virtual void procedureLoadDelta();
	virtual void procedureUpdate();
	virtual void procedureOutput();

// local logic functions
private:
	void clearMessages();

// handler helpers
private:
	//using callback_t = void (Worker::*)(const std::string&, const RPCInfo&);
	//typedef void (Worker::*callback_t)(const string&, const RPCInfo&);
	using typename Runner::callback_t;
	callback_t localCBBinder(void (Worker::*fp)(const std::string&, const RPCInfo&));
	virtual void registerHandlers();

// handlers
public:
	void handleReply(const std::string& d, const RPCInfo& info);

	void handleRegister(const std::string& d, const RPCInfo& info);
	void handleWorkers(const std::string& d, const RPCInfo& info);
	void handleShutdown(const std::string& d, const RPCInfo& info); // normal exit
	void handleTerminate(const std::string& d, const RPCInfo& info); // force exit

	void handleClear(const std::string& d, const RPCInfo& info);
	void handleProcedure(const std::string& d, const RPCInfo& info);
	void handleFinish(const std::string& d, const RPCInfo& info);

private:
	int master_net_id;
	WorkerIDMapper wm;
	
	GlobalHolder holder;

	std::thread tprcd; // thread for procedures
};
