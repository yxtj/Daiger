#pragma once
#include <string>
#include <vector>

class ArgumentSeparator;
class OperationBase;
class IOHandlerBase;
class TerminatorBase;
class GlobalHolderBase;

struct AppKernel {
	virtual ~AppKernel() = default;
	virtual std::string getName() const = 0;

	virtual void reg() = 0;

	virtual ArgumentSeparator* generateSeparator() = 0;
	virtual OperationBase* generateOperation() = 0;
	virtual IOHandlerBase* generateIOHandler() = 0;
	virtual TerminatorBase* generateTerminator() = 0;
	virtual GlobalHolderBase* generateGraph() = 0;
};
