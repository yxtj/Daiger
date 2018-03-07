#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include <vector>
#include <string>

class GlobalHolderBase{};

template <class V, class N>
class GlobalHolderBaseT
	: public GlobalHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using node_t = Node<V, N>;
	using value_t = node_t::value_t;
	using neighbor_t = node_t::neighbor_t;
	using neighbor_list_t = node_t::neighbor_list_t;

	void init(operation_t* opt, const size_t nPart);

	virtual void loadGraph(const std::string& fn);
	virtual void loadValue(const std::string& fn);
	virtual void loadDelta(const std::string& fn);
	virtual void update();
	virtual void output(const std::string& fn);

protected:
	virtual void receiveMessage(const std::string& d);

protected:
	operation* opt;
	size_t nPart;
};

template <class V, class N>
void GlobalHolderBaseT::init(operation_t* opt, const size_t nPart){
	this->opt = opt;
	this->nPart = nPart;
}

