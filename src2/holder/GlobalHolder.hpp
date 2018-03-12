#pragma once
#include "GlobalHolderBase.h"
#include "LocalHolder.hpp"
#include "RemoteHolder.hpp"
#include "msg/messages.h"
#include "serial/serialization.h"
#include <vector>
#include <string>

template <class V, class N>
class GlobalHolder
	: public GlobalHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using iohandler_t= IOHandler<V, N>;
	using node_t = Node<V, N>;
	using value_t = typename node_t::value_t;
	using neighbor_t = typename node_t::neighbor_t;
	using neighbor_list_t = typename node_t::neighbor_list_t;
	using msg_t = MessageDef<V>;

	virtual void init(OperationBase* opt, IOHandlerBase* ioh, SchedulerBase* scd, SharderBase* shd,
		const size_t nPart, const int local_id);

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
		return shd->owner(id);
	};
	bool is_local_part(const int pid){ return pid == local_id; }
	bool is_local_id(const id_t id){ return get_part(id) == local_id; }
	value_t get_ie() const { return IE; }
	node_t& get_node(const id_t id){ return local_part.get(id); }

	void add_local_node(id_t& id, neighbor_list_t& nl);

private:
	operation_t* opt;
	iohandler_t* ioh;
	SchedulerBase* scd;
	SharderBase* shd;
	size_t nPart;
	
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
	int local_id;
	value_t IE;

	int pointer_dump;
};

template <class V, class N>
void GlobalHolder<V, N>::init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, SharderBase* shd,
		const size_t nPart, const int local_id)
{
	this->opt = dynamic_cast<operation_t*>(opt);
	this->ioh = dynamic_cast<iohandler_t*>(ioh);
	this->scd = scd;
	this->shd = shd;
	this->nPart = nPart;
	this->local_id = local_id;
	IE = this->opt->identity_element();
	pointer_dump = 0;
}

template <class V, class N>
void GlobalHolder<V, N>::add_local_node(id_t& id, neighbor_list_t& nl){
	node_t n;
	n.id = std::move(id);
	n.v = n.u = get_ie();
	n.onb = std::move(nl);
	scd->regist(n.id);
	local_part.add(std::move(n));
}
template <class V, class N>
bool GlobalHolder<V, N>::loadGraph(const std::string& line){
	std::pair<id_t, neighbor_list_t> d = ioh->load_graph(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return false;
	add_local_node(d.first, d.second);
	return true;
}

template <class V, class N>
bool GlobalHolder<V, N>::loadValue(const std::string& line){
	std::pair<id_t, value_t> d = ioh->load_value(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return false;
	node_t& n = get_node(d.first);
	n.v = d.second;
	return true;
}

template <class V, class N>
bool GlobalHolder<V, N>::loadDelta(const std::string& line){
	ChangeEdge<N> d = ioh->load_change(line);
	int pid = get_part(d.src);
	if(!is_local_part(pid))
		return false;
	if(d.type == ChangeEdgeType::ADD){
		local_part.modify_onb_add(d.src, d.dst);
	}else if(d.type == ChangeEdgeType::REMOVE){
		local_part.modify_onb_rmv(d.src, d.dst);
	}else{
		local_part.modify_onb_val(d.src, d.dst);
	}
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
		return std::make_pair(true, ioh->dump_value(p->id, p->v));
	}else{
		return std::make_pair(false, "");
	}
}
template <class V, class N>
void GlobalHolder<V, N>::takeINCache(const std::string& line){
	auto ms = deserialize<typename msg_t::MsgGINCache_t>(line);
	for(typename msg_t::GINCache_t& m : ms) {
		local_part.update_cache(std::get<0>(m), std::get<1>(m), std::get<2>(m));
	}
}
template <class V, class N>
std::string GlobalHolder<V, N>::sendINCache(){
	// TODO:
	return "";
}

template <class V, class N>
void GlobalHolder<V, N>::msgUpdate(const std::string& line){
	auto ms = deserialize<typename msg_t::MsgVUpdate_t>(line);
	for(typename msg_t::VUpdate_t& m : ms) {
		local_part.cal_incremental(std::get<0>(m), std::get<1>(m), std::get<2>(m));
	}
}
template <class V, class N>
std::string GlobalHolder<V, N>::msgRequest(const std::string& line){
	auto m = deserialize<typename msg_t::VRequest_t>(line);
	value_t v = local_part.get(m.second).v;
	return serialize(typename msg_t::VReply_t{m.first, m.second, v});
}
template <class V, class N>
void GlobalHolder<V, N>::msgReply(const std::string& line){
	auto m = deserialize<typename msg_t::VReply_t>(line);
	local_part.cal_incremental(std::get<1>(m), std::get<0>(m), std::get<2>(m));
}
template <class V, class N>
std::string GlobalHolder<V, N>::msgSend(){
	// TODO:
	return "";
}

template <class V, class N>
void GlobalHolder<V, N>::applyChange(){

}

