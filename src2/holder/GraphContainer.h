#pragma once
#include <string>
#include "common/ConfData.h"
#include "application/AppBase.h"
#include "GlobalHolder.h"

class GraphContainer {
public:
	GraphContainer(AppBase& app, const ConfData& conf);
	~GraphContainer();
	void init();

	void loadGraph();
	void loadValue();
	void loadDelta();
	void update();
	void output();

private:
	AppBase app;
	const ConfData& conf;

	GlobalHolderBase* holder;
};

