#pragma once
#include "Runner.h"
#include "driver/tools/SyncUnit.h"
#include "runner_helpers.h"
#include "util/Timer.h"
#include <string>

class NetworkThread;

class Master : public Runner {
public:
	Master() = default;
	Master(AppBase& app, Option& opt);
	
	virtual void run();

protected:
	virtual void registerWorker();
	virtual void shutdownWorker(); // normal exit
	virtual void terminateWorker(); // force exit (error happened)

	virtual void procedureInit();
	virtual void procedureLoadGraph();
	virtual void procedureLoadValue();
	virtual void procedureLoadDelta();
	virtual void procedureBuildINCache();
	virtual void procedureGenIncrInitMsg();
	virtual void procedureUpdate();
	virtual void procedureDumpResult();

// local logic functions
private:
	int assignWid(const int nid); // bind to specific implementation
	void terminationCheck();
	void updateProgress(const int wid, const ProgressReport& report);

	void startProcedure(const int pid);
	void finishProcedure(const int pid);

// handlers
private:
	//using callback_t = void (Master::*)(const std::string&, const RPCInfo&);
	//typedef void (Master::*callback_t)(const string&, const RPCInfo&);
	using typename Runner::callback_t;
	callback_t localCBBinder(void (Master::*fp)(const std::string&, const RPCInfo&));
	virtual void registerHandlers();

public:
	void handleReply(const std::string& d, const RPCInfo& info);

	void handleRegister(const std::string& d, const RPCInfo& info);
	void handleProgressReport(const std::string& d, const RPCInfo& info);

private:
	int my_net_id;
	WorkerMonitor wm;
	int cpid; // current procedure id

	SyncUnit su_regw;
	SyncUnit su_procedure;
	SyncUnit su_term;
	SyncUnit su_clear;

	Timer tmr_procedure;
};
