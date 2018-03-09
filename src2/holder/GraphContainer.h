#pragma once
#include "common/ConfData.h"
#include "application/AppBase.h"
#include "GlobalHolderBase.h"
#include <string>

class GraphContainer {
public:
	GraphContainer(AppBase& app, const ConfData& conf);
	~GraphContainer();
	void init(int wid, GlobalHolderBase* holder);

	void loadGraph();
	void loadValue();
	void loadDelta();
	//void update();
	void dumpResult();
	void buildInNeighborCache();

// handlers:

	void loadGraphPiece(const std::string& line);
	void loadValuePiece(const std::string& line);
	void loadDeltaPiece(const std::string& line);

	void takeINCache(const std::string& line);
	std::string sendINCache();

	void msgUpdate(const std::string& line);
	void msgRequest(const std::string& line);
	void msgReply(const std::string& line);
	std::string msgSend();

private:
	void loadGraphFile(const std::string& fn);
	void loadValueFile(const std::string& fn);
	void loadDeltaFile(const std::string& fn);


private:
	AppBase app;
	const ConfData& conf;

	int wid;
	GlobalHolderBase* holder;
};

