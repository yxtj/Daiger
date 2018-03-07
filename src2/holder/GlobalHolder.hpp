#pragma once
#include "common/Node.h"
#include "api/api.h"
#include <vector>
#include <string>

class GlobalHolderBase{};

template <class V, class N>
class GlobalHolder
	: public GlobalHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using node_t = Node<V, N>;
	using value_t = node_t::value_t;
	using neighbor_t = node_t::neighbor_t;
	using neighbor_list_t = node_t::neighbor_list_t;

	void init(operation_t* opt, const size_t nPart);

	void loadGraph();
	void loadValue();
	void loadDelta();
	void update();
	void output();

private:
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
};