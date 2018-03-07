#pragma once
#include <string>
#include "common/ConfData.h"
#include "application/AppBase.h"

struct GlobalHolderImpl;

class GlobalHolder {
public:
	GlobalHolder(const AppBase& app, const ConfData& conf);
	~GlobalHolder();
	void init();

	void loadGraph();
	void loadValue();
	void loadDelta();
	void update();
	void output();

private:
	AppBase app;
	ConfData conf;
	GlobalHolderImpl* impl;
};

