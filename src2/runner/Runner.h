#pragma once
#include "application/AppBase.h"
#include "driver/MsgDriver.h"
#include "driver/tools/SyncUnit.h"
#include "driver/tools/ReplyHandler.h"
#include "network/RPCInfo.h"
#include "Option.h"
#include <string>
#include <thread>
#include <chrono>

class NetworkThread;

class Runner {
public:
	Runner() = default;
	Runner(AppBase& app, Option& opt);

	virtual void run() = 0;

protected:
	void msgLoop(const std::string& name = "");
	void sleep();
	void startMsgLoop(const std::string& name = "");
	void stopMsgLoop();
	void msgPausePush();
	void msgPausePop();
	void msgResumePush();
	void msgResumePop();
	void clearMessages();

	virtual void registerWorker() = 0;
	virtual void shutdownWorker() = 0;
	virtual void terminateWorker() = 0;

	virtual void procedureInit() = 0;
	virtual void procedureLoadGraph() = 0;
	virtual void procedureLoadValue() = 0;
	virtual void procedureLoadDelta() = 0;
	virtual void procedureBuildINCache() = 0;
	virtual void procedureRebuildStructure() = 0;
	virtual void procedureGenInitMsg() = 0;
	virtual void procedureUpdate() = 0;
	virtual void procedureDumpResult() = 0;

// handler helpers
protected:
	using callback_t = std::function<void(const std::string&, const RPCInfo&)>;
	//using callback_t = void (Master::*)(const std::string&, const RPCInfo&);
	//typedef void (Master::*callback_t)(const string&, const RPCInfo&);
	//using replier_t = std::function<void()>;
	virtual void registerHandlers() = 0;
	void regDSPImmediate(const int type, callback_t fp);
	void regDSPProcess(const int type, callback_t fp);
	void regDSPDefault(callback_t fp);

	void addRPHEach(const int type, std::function<void()> fun, const int n, const bool spawnThread=false);
	void addRPHEachSU(const int type, SyncUnit& su);
	void addRPHAny(const int type, std::function<void()> fun, const bool spawnThread=false);
	void addRPHAnySU(const int type, SyncUnit& su);

	void sendReply(const RPCInfo& info);

// handlers
public:
	// void handleReply(const std::string& d, const RPCInfo& info);

protected:
	AppBase app;
	Option opt;
	ReplyHandler rph;
	NetworkThread* net;

	std::chrono::duration<double> timeout; // cannot be float due to a g++ bug
	std::thread tmsg;
	MsgDriver driver;
	bool msg_do_push;
	bool msg_do_pop;
	bool running;
};
