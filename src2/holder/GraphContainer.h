#pragma once
#include "common/ConfData.h"
#include "application/AppBase.h"
#include "GlobalHolder.h"
#include <string>

class GraphContainer {
public:
	GraphContainer(AppBase& app, const ConfData& conf);
	~GraphContainer();
	void init(int wid, GlobalHolderBase* holder);

	void loadGraph();
	void loadValue();
	void loadDelta();
	void update();
	void output();

	void buildInNeighborCache();

private:
	void loadGraphFile(const std::string& fn);
	void loadValueFile(const std::string& fn);
	void loadDeltaFile(const std::string& fn);

	void loadGraphPiece(const std::string& line);
	void loadValuePiece(const std::string& line);
	void loadDeltaPiece(const std::string& line);
	
	void buildInNeighborCache(const std::string& line);

private:
	AppBase app;
	const ConfData& conf;

	int wid;
	GlobalHolderBase* holder;
};

