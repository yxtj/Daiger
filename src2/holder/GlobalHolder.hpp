#pragma once
#include "GlobalHolderBase.h"
#include "LocalHolder.hpp"
#include "RemoteHolder.hpp"
#include "msg/messages.h"
#include <vector>
#include <string>

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

	virtual void init(AppBase app, const size_t nPart, const int local_id);

	virtual bool loadGraph(const std::string& line);
	virtual bool loadValue(const std::string& line);
	virtual bool loadDelta(const std::string& line);
	virtual std::pair<bool, std::string> dumpResult(const std::string& line);

	virtual void takeINCache(const std::string& line);
	virtual std::string sendINCache();

	virtual void msgUpdate(const std::string& line);
	virtual void msgRequest(const std::string& line);
	virtual void msgReply(const std::string& line);
	virtual std::string msgSend();

private:
	int get_part(const id_t id){
		return app.shd->owner(id);
	};
	bool is_local_part(const int pid){ return pid == local_id; }
	bool is_local_id(const id_t id){ return get_part(id) == local_id; }
	value_t get_ie() const { return IE; }
	node_t& get_node(const id_t id){ return local_part.get(id); }

	void add_local_node(id_t& id, neighbor_list_t& nl);

private:
	AppBase app;
	size_t nPart;
	
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
	int local_id;
	value_t IE;

	int pointer_dump;
};

template <class V, class N>
void GlobalHolder<V, N>::init(AppBase app, const size_t nPart, const int local_id){
	this->app = app;
	this->nPart = nPart;
	this->local_id = local_id;
	IE = app.opt->identity_element();
	pointer_dump = 0;
}

template <class V, class N>
void GlobalHolder<V, N>::add_local_node(id_t& id, neighbor_list_t& nl){
	node_t n;
	n.id = move(id);
	n.v = n.u = get_ie();
	n.neighbor_list_t = move(nl);
	local_part.add(move(n));
}
template <class V, class N>
bool GlobalHolder<V, N>::loadGraph(const std::string& line){
	std::pair<id_t, neighbor_list_t> d = app.ioh->load_graph(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return false;
	add_local_node(d.first, d.second);
	return true;
}

template <class V, class N>
bool GlobalHolder<V, N>::loadValue(const std::string& line){
	std::pair<id_t, value_t> d = app.ioh->load_value(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return false;
	node_t& n = get_node(d.first);
	n.v = d.second;
	return true;
}

template <class V, class N>
bool GlobalHolder<V, N>::loadDelta(const std::string& line){
	
}

template <class V, class N>
std::pair<bool, std::string> GlobalHolder<V, N>::dumpResult(){

}