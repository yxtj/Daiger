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
	using msg_t = MessageDef<V>;

	virtual void init(AppBase app, const size_t nPart, const int local_id);

	virtual bool loadGraph(const std::string& line);
	virtual bool loadValue(const std::string& line);
	virtual bool loadDelta(const std::string& line);
	virtual void prepareDump();
	virtual std::pair<bool, std::string> dumpResult();

	virtual void takeINCache(const std::string& line);
	virtual std::string sendINCache();

	virtual void msgUpdate(const std::string& line);
	virtual std::string msgRequest(const std::string& line);
	virtual void msgReply(const std::string& line);
	virtual std::string msgSend();

	virtual void applyChange();

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
	app.scd->regist(n.id);
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
	std::pair<id_t, value_t> d = app.ioh->load_delta(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return false;
	node_t& n = get_node(d.first);
	n.v = d.second;
	return true;
}

template <class V, class N>
void GlobalHolder<V, N>::prepareDump(){
	local_part.enum_rewind();
}
template <class V, class N>
std::pair<bool, std::string> GlobalHolder<V, N>::dumpResult(){
	const node_t* p = local_part.enum_next();
	if(p != nullptr){
		return std::make_pair(true, app.ioh->dump_value(p->id, p->v));
	}else{
		return std::make_pair(false, "");
	}
}
template <class V, class N>
void GlobalHolder<V, N>::takeINCache(const std::string& line){
	msg_t::MsgGINCache_t ms = deserialize<msg_t::MsgGINCache_t>(line);
	for(msg_t::GINCache_t& m : ms) {
		local_part.update_cache(std::get<0>(m), std::get<1>(m), std::get<2>(m));
	}
}
template <class V, class N>
std::string void GlobalHolder<V, N>::sendINCache(){
	// TODO:
}

template <class V, class N>
void GlobalHolder<V, N>::msgUpdate(const std::string& line){
	msg_t::MsgVUpdate_t ms = deserialize<msg_t::MsgVUpdate_t>(line);
	for(msg_t::VUpdate_t& m : ms) {
		local_part.cal_incremental(std::get<0>(m), std::get<1>(m), std::get<2>(m));
	}
}
template <class V, class N>
std::string GlobalHolder<V, N>::msgRequest(const std::string& line){
	msg_t::VRequest_t m = deserialize<msg_t::VRequest_t>(line);
	value_t v = local_part.get(m.second);
	return serialize(msg_t::VReply_t{m.first, m.second, v});
}
template <class V, class N>
void GlobalHolder<V, N>::msgReply(const std::string& line){
	msg_t::VReply_t m = deserialize<msg_t::VReply_t>(line);
	local_part.cal_incremental(std::get<1>(m), std::get<0>(m), std::get<2>(m));
}
template <class V, class N>
std::string GlobalHolder<V, N>::msgSend(){

}

template <class V, class N>
void GlobalHolder<V, N>::applyChange(){

}

