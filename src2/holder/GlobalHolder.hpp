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
#ifndef NDEBUG
#include "dbg/dbg.h"
#include "logging/logging.h"
#endif

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
		SchedulerBase* scd, PartitionerBase* ptn, TerminatorBase* tmt,
		const size_t nPart, const int localId, const size_t send_batch_size,
		const bool incremental, const bool async, const bool cache_free, const bool sort_result);

	virtual int loadGraph(const std::string& line);
	virtual int loadValue(const std::string& line);
	virtual int loadDelta(const std::string& line);
	virtual void prepareUpdate(sender_t f_req);
	virtual void prepareCollectINCache();
	virtual void intializedProcess();
	virtual void prepareDump();
	virtual std::pair<bool, std::string> dumpResult();

	virtual void addDummyNodes();

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
		return ptn->owner(id);
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
	PartitionerBase* ptn;
	terminator_t* tmt;
	size_t nPart;
	size_t send_batch_size;

	bool incremental;
	bool async;
	bool cache_free;
	bool sort_result;
	
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
	int local_id;

	int pointer_dump;
	bool applying;
	bool sending;
};

template <class V, class N>
void GlobalHolder<V, N>::init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, PartitionerBase* ptn, TerminatorBase* tmt,
		const size_t nPart, const int localId, const size_t send_batch_size,
		const bool incremental, const bool async, const bool cache_free, const bool sort_result)
{
	this->opt = dynamic_cast<operation_t*>(opt);
	this->ioh = dynamic_cast<iohandler_t*>(ioh);
	this->scd = scd;
	this->ptn = ptn;
	this->tmt = dynamic_cast<terminator_t*>(tmt);
	this->nPart = nPart;
	this->local_id = localId;
	this->send_batch_size = send_batch_size;
	this->incremental = incremental;
	this->async = async;
	this->cache_free = cache_free;
	this->sort_result = sort_result;

	this->ptn->setParts(nPart);
	pointer_dump = 0;

	local_part.init(this->opt, this->scd, this->tmt, nPart, incremental, async, cache_free);
	remote_parts.resize(nPart);
	for(size_t i = 1; i<nPart; ++i){
		remote_parts[i].init(this->opt);
	}
	applying = false;
}

template <class V, class N>
void GlobalHolder<V, N>::add_local_node(id_t& id, neighbor_list_t& nl){
	scd->regist(id);
	local_part.add(opt->preprocess_node(id, nl));
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
void GlobalHolder<V, N>::addDummyNodes(){
	std::vector<typename operation_t::DummyNode> dummies = opt->dummy_nodes();
	for(auto& p : dummies){
		id_t id = p.node.id;
		if(p.type == DummyNodeType::NORMAL){
			// only add to its owner worker
			if(is_local_id(id)){
				scd->regist(id);
				local_part.add(std::move(p.node));
				local_part.modify_onb_via_fun_all(id, p.func);
			}
		}else if(p.type == DummyNodeType::TO_ALL){
			// add to all workers, and each one only connects to its local nodes
			scd->regist(id);
			local_part.add(std::move(p.node));
			local_part.modify_onb_via_fun_all(id, p.func);
		}
	}
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
void GlobalHolder<V, N>::intializedProcess(){
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true); p != nullptr; p = local_part.enum_next(true)){
		local_part.cal_general(p->id);
	}
}

template <class V, class N>
void GlobalHolder<V, N>::prepareUpdate(sender_t f_req){
	scd->ready();
	//local_part.registerRequestCallback(f_req);
	local_part.registerRequestCallback([=](const id_t& rf, const id_t& rt){
		int pid = this->get_part(rt);
		std::string msg = serialize<typename msg_t::VRequest_t>(std::make_pair(rf, rt));
		f_req(pid, msg);
	});
}
template <class V, class N>
void GlobalHolder<V, N>::prepareCollectINCache(){
	local_part.enum_rewind();
}
template <class V, class N>
void GlobalHolder<V, N>::prepareDump(){
	if(sort_result){
		local_part.enum_sorted_prepare();
		local_part.enum_sorted_rewind();
	}else{
		local_part.enum_rewind();
	}
}
template <class V, class N>
std::pair<bool, std::string> GlobalHolder<V, N>::dumpResult(){
	const node_t* p;
	if(sort_result)
		p = local_part.enum_sorted_next();
	else
		p = local_part.enum_next();
	if(p != nullptr){
		V v = opt->postprocess_value(*p);
		return std::make_pair(true, ioh->dump_value(p->id, v));
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
	size_t bs = 0;
	msgs.reserve(nPart);
	for(const node_t* p = local_part.enum_next(true); bs < send_batch_size && p != nullptr; p = local_part.enum_next(true)){
		for(auto& nb : p->onb){
			id_t dst = get_key(nb);
			int pid = get_part(dst);
			value_t v = opt->func(*p, nb);
			if(is_local_part(pid)){
				local_part.update_cache(p->id, dst, v);
			}else{
				msgs[pid].emplace_back(p->id, dst, v);
				++bs;
			}
		}
	}
	std::unordered_map<int, std::string> res;
	res.reserve(nPart);
	for(auto& mp : msgs){
		if(!mp.second.empty())
			res[mp.first]=serialize(mp.second);
		mp.second.clear();
	}
	return res;
}

template <class V, class N>
void GlobalHolder<V, N>::local_update_cal(const id_t& from, const id_t& to, const value_t& v){
	local_part.cal_incremental(from, to, v);
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
	return !applying && !scd->empty();
		//local_part.has_uncommitted();
}
template <class V, class N>
void GlobalHolder<V, N>::doApply(){
	applying = true;
	std::vector<id_t> nodes = scd->pick();
	for(id_t id : nodes){
		#ifndef NDEBUG
		const node_t& n = local_part.get(id);
		DVLOG(3)<<"k="<<n.id<<" v="<<n.v<<" u="<<n.u<<" cache="<<n.cs;
		auto pgs = local_part.get_progress();
		DVLOG(3)<<"progress=("<<pgs.sum<<","<<pgs.n_inf<<","<<pgs.n_change<<") update="<<local_part.get_n_uncommitted();
		#endif
		if(!local_part.commit(id))
			continue;
		std::vector<std::pair<id_t, value_t>> data = local_part.spread(id);
		DVLOG(3)<<data;
		for(auto& p : data){
			int pid = get_part(p.first);
			if(is_local_part(pid)){
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


