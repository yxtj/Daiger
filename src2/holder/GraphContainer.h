#pragma once
#include "common/ConfData.h"
#include "application/AppBase.h"
#include "GlobalHolderBase.h"
#include <functional>
#include <unordered_map>
#include <string>

class GraphContainer {
public:
	GraphContainer(AppBase& app, const ConfData& conf);
	void init(int wid, GlobalHolderBase* holder, bool incremental);

	using sender_t = std::function<void(const int, std::string&)>;
	using sender0_t = std::function<void(std::string&)>;

	// the std::function<void(const int, std::string&)> sender is used to send the messages to other WORKERS
	void loadGraph(sender_t sender = {});
	void loadValue(sender_t sender = {});
	void loadDelta(sender_t sender = {});
	void buildINCache(sender_t sender);
	void genIncrInitMsg();
	void prepareUpdate(sender_t sender_val, sender_t sender_req, sender0_t sender_pro);
	void dumpResult();

// handlers:

	bool loadGraphPiece(const std::string& line);
	bool loadValuePiece(const std::string& line);
	bool loadDeltaPiece(const std::string& line);

	void takeINCache(const std::string& line);

	void msgUpdate(const std::string& line);
	void msgRequest(const std::string& line);
	void msgReply(const std::string& line);

	void apply();
	void send();
	void reportProgress();

private:
	void loadGraphFile(const std::string& fn, sender_t sender);
	void loadValueFile(const std::string& fn, sender_t sender);
	void loadDeltaFile(const std::string& fn, sender_t sender);

	bool needApply();

private:
	AppBase app;
	const ConfData& conf;

	int wid;
	GlobalHolderBase* holder;
	sender_t sender_val;
	sender_t sender_req;
	sender0_t sender_pro;
	bool applying;
	bool sending;
};

