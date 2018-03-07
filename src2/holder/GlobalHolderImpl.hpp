#pragma once
#include "GlobalHolder.h"
#include "LocalHolder.hpp"
#include "RemoteHolder.hpp"
#include <vector>
#include <string>

class GlobalHolderBase{};

template <class V, class N>
class GlobalHolderImpl
	: public GlobalHolderBaseT<V, N>
{
	using parent_t = GlobalHolderBaseT<V, N>;
public:
	using operation_t = typename parent_t::operation_t;
	using node_t = typename parent_t::node_t;
	using value_t = typename parent_t::value_t;
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
	virtual void applyChanges();

private:
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
};