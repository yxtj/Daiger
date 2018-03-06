#pragma once
#include "Runner.h"
#include <string>

class NetworkThread;

class Master : public Runner {
public:
	Master() = default;
	Master(const AppBase& app, Option& opt);
	
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
	//using callback_t = void (Master::*)(const std::string&, const RPCInfo&);
	//typedef void (Master::*callback_t)(const string&, const RPCInfo&);
	using typename Runner::callback_t;
	callback_t localBinder(void (Master::*fp)(const std::string&, const RPCInfo&));
	virtual void registerHandlers();

	void handleRegister(const std::string& d, const RPCInfo& info);
	void handleProgressReport(const std::string& d, const RPCInfo& info);

private:

};
