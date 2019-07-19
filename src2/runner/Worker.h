#pragma once
#include "Runner.h"
#include "network/RPCInfo.h"
#include "runner_helpers.h"
#include "holder/GraphContainer.h"
#include <string>
#include <vector>
#include <thread>

class NetworkThread;

class Worker : public Runner {
public:
    Worker() = default;
    Worker(AppBase& app, Option& opt);
	
	virtual void run();

protected:
	virtual void registerWorker();
	virtual void shutdownWorker();
	virtual void terminateWorker();

	virtual void procedureInit();
	virtual void procedureLoadGraph();
	virtual void procedureLoadValue();
	virtual void procedureLoadDelta();
	virtual void procedureBuildINList();
	virtual void procedureBuildINCache();
	virtual void procedureRebuildStructure();
	virtual void procedureGenInitMsg();
	virtual void procedureUpdate();
	virtual void procedureDumpResult();

// local logic functions
private:
	void storeWorkerInfo(const std::vector<std::pair<int, int>>& winfo);
	void reportProgress();

// handler helpers
private:
	//using callback_t = void (Worker::*)(const std::string&, const RPCInfo&);
	//typedef void (Worker::*callback_t)(const string&, const RPCInfo&);
	using typename Runner::callback_t;
	callback_t localCBBinder(void (Worker::*fp)(std::string&, const RPCInfo&));
	virtual void registerHandlers();

// handlers
public:
	void handleReply(std::string& d, const RPCInfo& info);

	void handleOnline(std::string& d, const RPCInfo& info);
	void handleRegister(std::string& d, const RPCInfo& info);
	void handleWorkers(std::string& d, const RPCInfo& info);
	void handleShutdown(std::string& d, const RPCInfo& info); // normal exit
	void handleTerminate(std::string& d, const RPCInfo& info); // force exit

	void handleClear(std::string& d, const RPCInfo& info);
	void handleProcedure(std::string& d, const RPCInfo& info);
	void handleFinish(std::string& d, const RPCInfo& info);

	void handleGNode(std::string& d, const RPCInfo& info);
	void handleGValue(std::string& d, const RPCInfo& info);
	void handleGDelta(std::string& d, const RPCInfo& info);

	void handleINCache(std::string& d, const RPCInfo& info);

	void handleVUpdate(std::string& d, const RPCInfo& info);
	void handleVRequest(std::string& d, const RPCInfo& info);
	void handleVReply(std::string& d, const RPCInfo& info);

	//void handlePApply(std::string& d, const RPCInfo& info);
	//void handlePSend(std::string& d, const RPCInfo& info);
	//void handlePReport(std::string& d, const RPCInfo& info);
	void handlePFinish(std::string& d, const RPCInfo& info);

private:
	int master_net_id;
	int my_net_id;
	WorkerIDMapper wm;
	std::string log_name;
	
	GraphContainer graph;

	std::thread tprcd; // thread for procedures

	SyncUnit su_master;
	SyncUnit su_regw;
	SyncUnit su_winfo;
	bool update_finish;
	SyncUnit su_update;

	SyncUnit su_stop;
};
