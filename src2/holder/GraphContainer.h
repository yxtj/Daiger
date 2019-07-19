#pragma once
#include "common/ConfData.h"
#include "application/AppBase.h"
#include "GlobalHolderBase.h"
#include "util/Timer.h"
#include <functional>
#include <mutex>
#include <deque>
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
	void buildINList(sender_t sender);
	void buildINCache(sender_t sender);
	void rebuildSource();
	void clearINCache();
	void genInitMsg();
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

	// do the update loop, including: process message, apply, send, report progress.
	void update();
	enum class MsgType {
		Update,
		Request,
		Reply
	};
	void pushMsg(MsgType type, std::string & msg);
	void stop_update();

	void apply(); // apply local u to local v
	void tryApply();
	void send(); // send remote u to their workers
	void trySend();
	void report(); // report local progress to master
	void tryReport();

private:
	void loadGraphFile(const std::string& fn, sender_t sender);
	void loadValueFile(const std::string& fn, sender_t sender);
	void loadDeltaFile(const std::string& fn, sender_t sender);

	std::pair<MsgType, std::string> popMsg(); // only called when message queue is not empty

private:
	AppBase app;
	const ConfData& conf;

	int wid;
	GlobalHolderBase* holder;
	sender_t sender_val;
	sender_t sender_req;
	sender0_t sender_pro;
	
	bool allow_update;
	bool applying;
	bool sending;
	std::mutex mtx;
	std::deque<std::pair<MsgType, std::string>> messages; // buffered messages to be processed in update()

	Timer tmr;
	double t_last_apply;
	double t_last_send;
	double t_last_report;
};

