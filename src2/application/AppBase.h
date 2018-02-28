#pragma once

class KernelBase;
class TerminatorBase;
class SharderBase;
class SchedulerBase;
class IOHanderBase;

class AppBase{
public:
	virtual KernelBase* getKernel() = 0;
	virtual TerminatorBase* getTerminator() = 0;
	virtual SharderBase* getShader() = 0;
	virtual SchedulerBase* getScheduler() = 0;
	virtual IOHanderBase* getIOHander() = 0;
};
