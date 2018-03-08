#pragma once
#include "common/Node.h"
#include "application/Operation.h"
#include <vector>
#include <string>

class GlobalHolderBase{
public:
	virtual void init(OperationBase* opt, const size_t nPart);

	virtual void loadGraph(const std::string& line);
	virtual void loadValue(const std::string& line);
	virtual void loadDelta(const std::string& line);

	virtual void output(const std::string& line);

	virtual void msgUpdate(const std::string& line);
	virtual void msgRequest(const std::string& line);
	virtual void msgReply(const std::string& line);
};

template <class V, class N>
class GlobalHolderImpl;

template <class V, class N>
class GlobalHolder
	: public GlobalHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using node_t = Node<V, N>;
	using value_t = typename node_t::value_t;
	using neighbor_t = typename node_t::neighbor_t;
	using neighbor_list_t = typename node_t::neighbor_list_t;

	virtual void init(operation_t* opt, const size_t nPart);

	virtual void loadGraph(const std::string& line);
	virtual void loadValue(const std::string& line);
	virtual void loadDelta(const std::string& line);
	
	virtual void output(const std::string& line);

	virtual void msgUpdate(const std::string& line);
	virtual void msgRequest(const std::string& line);
	virtual void msgReply(const std::string& line);

protected:
	operation_t* opt;
	GlobalHolderImpl<V, N>* impl;
	size_t nPart;
};

template <class V, class N>
void GlobalHolder<V, N>::init(operation_t* opt, const size_t nPart){
	this->opt = opt;
	this->nPart = nPart;
}

