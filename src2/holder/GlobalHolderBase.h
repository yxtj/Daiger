#pragma once
#include "application/Operation.h"
#include "application/IOHandler.h"
#include "application/Scheduler.h"
#include "application/Sharder.h"
#include "application/Terminator.h"
#include <vector>
#include <string>
#include <functional>

class GlobalHolderBase{
public:
	using sender_t = std::function<void(const int, std::string&)>;

	virtual ~GlobalHolderBase() = default;

	virtual void init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, SharderBase* shd, TerminatorBase* tmt,
		const size_t nPart, const int localId, const size_t send_batch_size,
		const bool incremental, const bool cache_free, const bool sort_result) = 0;

	// IO (loadXXXX returns the part-id (worker-id) should have the input line)
	virtual int loadGraph(const std::string& line) = 0;
	virtual int loadValue(const std::string& line) = 0;
	virtual int loadDelta(const std::string& line) = 0;
	virtual void prepareUpdate(sender_t f_req) = 0;
	virtual void prepareDump() = 0;
	// return whether this call is success and the line to write
	virtual std::pair<bool, std::string> dumpResult() = 0;

	virtual void addDummyNodes() = 0;

	// in-neighbor cache
	virtual void takeINCache(const std::string& line) = 0;
	virtual std::unordered_map<int, std::string> collectINCache() = 0;

	// update mesages handler
	virtual void msgUpdate(const std::string& line) = 0;
	virtual std::string msgRequest(const std::string& line) = 0;
	virtual void msgReply(const std::string& line) = 0;
	// update procedure
	virtual bool needApply() = 0;
	virtual void doApply() = 0;
	// update sending
	virtual bool needSend(bool force = false) = 0;
	virtual std::string collectMsg(const int pid) = 0;

	virtual std::string collectLocalProgress() = 0;
};
