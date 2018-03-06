#pragma once
#include "application/AppBase.h"
#include "driver/MsgDriver.h"
#include "driver/tools/ReplyHandler.h"
#include "network/RPCInfo.h"
#include "Option.h"
#include <string>
#include <thread>

class NetworkThread;

class Runner {
public:
	Runner() = default;
	Runner(const AppBase& app, Option& opt);

	virtual void start() = 0;
	virtual void finish() = 0;

protected:
	void msgLoop();
	void sleep();
	void start_running();
	void stop_running();

	virtual void registerWorker() = 0;
	virtual void terminateWorker() = 0;

	virtual void procedureLoadGraph() = 0;
	virtual void procedureLoadValue() = 0;
	virtual void procedureLoadDelta() = 0;
	virtual void procedureUpdate() = 0;
	virtual void procedureOutput() = 0;

// handlers
protected:
	using callback_t = std::function<void(const std::string&, const RPCInfo&)>;
	//using callback_t = void (Master::*)(const std::string&, const RPCInfo&);
	//typedef void (Master::*callback_t)(const string&, const RPCInfo&);
	//using replier_t = std::function<void()>;
	virtual void registerHandlers() = 0;
	void addReplyHandler(const int mtype, std::function<void()> fun,const bool spwanThread=false);
	void regDSPImmediate(const int type, callback_t fp);
	void regDSPProcess(const int type, callback_t fp);
	void regDSPDefault(callback_t fp);

	void handleReply(const std::string& d, const RPCInfo& info);

protected:
	AppBase app;
	Option opt;
	MsgDriver driver;
	ReplyHandler rph;
	NetworkThread* net;

	std::thread tmsg;
	bool running;
};
