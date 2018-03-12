#pragma once
#include "common/Node.h"
#include "application/AppBase.h"
#include <vector>
#include <string>

class GlobalHolderBase{
public:
	virtual void init(AppBase app, const size_t nPart, const int local_id) = 0;

	// IO (loadXXXX returns whether this line is accepted locally)
	virtual bool loadGraph(const std::string& line) = 0;
	virtual bool loadValue(const std::string& line) = 0;
	virtual bool loadDelta(const std::string& line) = 0;

	virtual void prepareDump() = 0;
	// return whether this call is success and the line to write
	virtual std::pair<bool, std::string> dumpResult() = 0;

	// in-neighbor cache
	virtual void takeINCache(const std::string& line) = 0;
	virtual std::string sendINCache() = 0;

	// update mesages
	virtual void msgUpdate(const std::string& line) = 0;
	virtual std::string msgRequest(const std::string& line) = 0;
	virtual void msgReply(const std::string& line) = 0;
	virtual std::string msgSend() = 0;

	// update procedure
};
