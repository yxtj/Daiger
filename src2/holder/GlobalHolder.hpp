#pragma once
#include "GlobalHolderBase.h"
#include "common/Node.h"
#include "LocalHolder.hpp"
#include "RemoteHolder.hpp"
#include "msg/messages.h"
#include "serial/serialization.h"
#include <vector>
#include <string>
#include <functional>

template <class V, class N>
class GlobalHolder
	: public GlobalHolderBase
{
public:
	using operation_t = Operation<V, N>;
	using iohandler_t= IOHandler<V, N>;
	using terminator_t = Terminator<V, N>;
	using node_t = Node<V, N>;
	using value_t = typename node_t::value_t;
	using neighbor_t = typename node_t::neighbor_t;
	using neighbor_list_t = typename node_t::neighbor_list_t;
	using sender_t = std::function<void(const int, std::string&)>;
	using msg_t = MessageDef<V>;

	virtual void init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, SharderBase* shd, TerminatorBase* tmt,
		const size_t nPart, const int localId, const size_t send_batch_size,
		const bool incremental, const bool cache_free, const bool localProcess);

	virtual int loadGraph(const std::string& line);
	virtual int loadValue(const std::string& line);
	virtual int loadDelta(const std::string& line);
	virtual void prepareUpdate(sender_t f_req);
	virtual void prepareDump();
	virtual std::pair<bool, std::string> dumpResult();

	virtual void takeINCache(const std::string& line);
	virtual std::unordered_map<int, std::string> collectINCache();

	virtual void msgUpdate(const std::string& line);
	virtual std::string msgRequest(const std::string& line);
	virtual void msgReply(const std::string& line);

	virtual bool needApply();
	virtual void doApply();
	virtual bool needSend(bool force);
	virtual std::string collectMsg(const int pid);

	virtual std::string collectLocalProgress();

private:
	int get_part(const id_t id){
		return shd->owner(id);
	};
	bool is_local_part(const int pid){ return pid == local_id; }
	bool is_local_id(const id_t id){ return get_part(id) == local_id; }
	node_t& get_node(const id_t id){ return local_part.get(id); }

	void add_local_node(id_t& id, neighbor_list_t& nl);

	void local_update_cal(const id_t& from, const id_t& to, const value_t& v);

private:
	operation_t* opt;
	iohandler_t* ioh;
	SchedulerBase* scd;
	SharderBase* shd;
	terminator_t* tmt;
	size_t nPart;
	size_t send_batch_size;
	bool incremental;
	bool cache_free;
	bool enable_local_process;
	
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
	int local_id;

	int pointer_dump;
	bool applying;
	bool sending;
};

template <class V, class N>
void GlobalHolder<V, N>::init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, SharderBase* shd, TerminatorBase* tmt,
		const size_t nPart, const int localId, const size_t send_batch_size,
		const bool incremental, const bool cache_free, const bool localProcess)
{
	this->opt = dynamic_cast<operation_t*>(opt);
	this->ioh = dynamic_cast<iohandler_t*>(ioh);
	this->scd = scd;
	this->shd = shd;
	this->tmt = dynamic_cast<terminator_t*>(tmt);
	this->nPart = nPart;
	this->local_id = localId;
	this->send_batch_size = send_batch_size;
	this-> incremental = incremental;
	this-> cache_free = cache_free;
	this->enable_local_process = localProcess;

	this->shd->setParts(nPart);
	pointer_dump = 0;

	local_part.init(this->opt, this->scd, this->tmt, nPart);
	remote_parts.resize(nPart);
	for(size_t i = 1; i<nPart; ++i){
		remote_parts[i].init(this->opt);
	}
	applying = false;
}

template <class V, class N>
void GlobalHolder<V, N>::add_local_node(id_t& id, neighbor_list_t& nl){
	node_t n;
	n.id = std::move(id);
	n.u = opt->init_value(id, nl);
	n.v = opt->identity_element();
	n.onb = std::move(nl);
	scd->regist(n.id);
	local_part.add(std::move(n));
}
template <class V, class N>
int GlobalHolder<V, N>::loadGraph(const std::string& line){
	std::pair<id_t, neighbor_list_t> d = ioh->load_graph(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return pid;
	add_local_node(d.first, d.second);
	return pid;
}

template <class V, class N>
int GlobalHolder<V, N>::loadValue(const std::string& line){
	std::pair<id_t, value_t> d = ioh->load_value(line);
	int pid = get_part(d.first);
	if(!is_local_part(pid))
		return pid;
	node_t& n = get_node(d.first);
	n.v = d.second;
	return pid;
}

template <class V, class N>
int GlobalHolder<V, N>::loadDelta(const std::string& line){
	ChangeEdge<N> d = ioh->load_change(line);
	int pid = get_part(d.src);
	if(!is_local_part(pid))
		return pid;
	if(d.type == ChangeEdgeType::ADD){
		local_part.modify_onb_add(d.src, d.dst);
	}else if(d.type == ChangeEdgeType::REMOVE){
		local_part.modify_onb_rmv(d.src, d.dst);
	}else{
		local_part.modify_onb_val(d.src, d.dst);
	}
	return pid;
}

template <class V, class N>
void GlobalHolder<V, N>::prepareUpdate(sender_t f_req){
	scd->ready();
	local_part.registerRequestCallback(f_req);
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
		return std::make_pair(false, std::string());
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
std::unordered_map<int, std::string> GlobalHolder<V, N>::collectINCache(){
	// vector<vector<Msg>> : for each worker, for each unit
	std::unordered_map<int, typename msg_t::MsgGINCache_t> msgs;
	msgs.reserve(nPart);
	local_part.enum_rewind();
	const node_t* p = local_part.enum_next();
	while(p != nullptr){
		for(auto& nb : p->onb){
			id_t dst = get_key(nb);
			int pid = get_part(dst);
			value_t v = opt->func(*p, nb);
			if(enable_local_process && is_local_part(pid)){
				local_part.update_cache(p->id, dst, v);
			}else{
				msgs[pid].emplace_back(p->id, dst, v);
			}
		}
		p = local_part.enum_next();
	}
	std::unordered_map<int, std::string> res;
	res.reserve(nPart);
	for(auto& mp : msgs){
		if(!mp.second.empty())
			res[mp.first]=serialize(mp.second);
	}
	return res;
}

template <class V, class N>
void GlobalHolder<V, N>::local_update_cal(const id_t& from, const id_t& to, const value_t& v){
	if(incremental)
		local_part.cal_incremental(from, to, v);
	else
		local_part.cal_nonincremental(from, to, v);
}

template <class V, class N>
void GlobalHolder<V, N>::msgUpdate(const std::string& line){
	auto ms = deserialize<typename msg_t::MsgVUpdate_t>(line);
	for(typename msg_t::VUpdate_t& m : ms) {
		local_update_cal(std::get<0>(m), std::get<1>(m), std::get<2>(m));
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
bool GlobalHolder<V, N>::needApply(){
	return !applying;
}
template <class V, class N>
void GlobalHolder<V, N>::doApply(){
	applying = true;
	std::vector<id_t> nodes = scd->pick();
	for(id_t id : nodes){
		local_part.commit(id);
		std::vector<std::pair<id_t, value_t>> data = local_part.spread(id);
		for(auto& p : data){
			int pid = get_part(p.first);
			if(enable_local_process && is_local_part(pid)){
				local_update_cal(id, p.first, p.second);
			}else{
				remote_parts[pid].update(id, p.first, p.second);
			}
		}
	}
	applying = false;
}
template <class V, class N>
bool GlobalHolder<V, N>::needSend(bool force){
	if(sending)
		return false;
	size_t th = force ? 0 : send_batch_size;
	size_t sum = 0;
	for(auto& t : remote_parts){
		sum += t.size();
		if(sum > th)
			return true;
	}
	return false;
}
template <class V, class N>
std::string GlobalHolder<V, N>::collectMsg(const int pid){
	sending = true;
	std::vector<std::pair<id_t, std::pair<id_t, value_t>>> data = remote_parts[pid].collect();
	std::string res = serialize(data);
	sending = false;
	return res;
}

template <class V, class N>
std::string GlobalHolder<V, N>::collectLocalProgress(){
	ProgressReport progress = local_part.get_progress();
	local_part.reset_progress_count();
	return serialize(progress);
}


