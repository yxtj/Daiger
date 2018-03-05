#pragma once
#include "api/api.h"
#include "application/AppBase.h"
#include "driver/MsgDriver.h"
#include "driver/tools/ReplyHandler.h"
#include "network/RPCInfo.h"
#include "Option.h"
#include <string>
#include <thread>

class NetworkThread;

class Master {
public:
	Master() = default;
	Master(const AppBase& app, Option& opt);
	
	void start();
	void finish();

private:
	void msgLoop();
	void sleep();

	void registerWorkers();
	void terminateWorkers();

	void procedureLoadGraph();
	void procedureLoadValue();
	void procedureLoadDelta();
	void procedureUpdate();
	void procedureOutput();

// handlers
private:
	using callback_t = void (Master::*)(const std::string&, const RPCInfo&);
	//typedef void (Master::*callback_t)(const string&, const RPCInfo&);
	void registerHandlers();
	void addReplyHandler(const int mtype, void (Master::*fp)(),const bool spwanThread=false);
	void regDSPImmediate(const int type, callback_t fp);
	void regDSPProcess(const int type, callback_t fp);
	void regDSPDefault(callback_t fp);

	void handleReply(const std::string& d, const RPCInfo& info);
	void handleRegister(const std::string& d, const RPCInfo& info);

	void handleProgressReport(const std::string& d, const RPCInfo& info);

private:
	AppBase app;
	Option opt;
	MsgDriver driver;
	ReplyHandler rph;
	NetworkThread* net;

	std::thread tmsg;
	bool running;
};
