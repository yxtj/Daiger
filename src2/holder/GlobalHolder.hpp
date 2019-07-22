#pragma once
#include "GlobalHolderBase.h"
#include "common/Node.h"
#include "LocalHolder.hpp"
#include "RemoteHolder.hpp"
#include "msg/messages.h"
#include "serial/serialization.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#if !defined(NDEBUG) || defined(_DEBUG)
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
	using progressor_t = Progressor<V, N>;
	using node_t = Node<V, N>;
	using value_t = typename node_t::value_t;
	using neighbor_t = typename node_t::neighbor_t;
	using neighbor_list_t = typename node_t::neighbor_list_t;
	using sender_t = std::function<void(const int, std::string&)>;
	using msg_t = MessageDef<V, N>;

	virtual void init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, PartitionerBase* ptn, ProgressorBase* prg,
		const size_t nPart, const int localId, const bool aggregate_message,
		const bool incremental, const bool async, const bool cache_free, const bool sort_result);

	virtual size_t numLocalNode();

	virtual int loadGraph(const std::string& line);
	virtual int loadValue(const std::string& line);
	virtual int loadDelta(const std::string& line);
	virtual void prepareUpdate(sender_t f_req);
	virtual void prepareCollectINList();
	virtual void prepareCollectINCache();
	virtual void rebuildSource(); // for selective operators
	virtual void intializedProcess(); // update <u> and put nodes into the scheduler
	virtual void prepareDump();
	virtual std::pair<bool, std::string> dumpResult();

	virtual void addDummyNodes();

	// in-neighbor
	virtual void clearINList();
	virtual void takeINList(const std::string& line);
	virtual std::unordered_map<int, std::string> collectINList();

	virtual void clearINCache();
	virtual void takeINCache(const std::string& line);
	virtual std::unordered_map<int, std::string> collectINCache();
	virtual std::string collectINCache(const size_t pid);

	virtual void msgUpdate(const std::string& line);
	virtual std::string msgRequest(const std::string& line);
	virtual void msgReply(const std::string& line);

	virtual size_t toApply();
	virtual void doApply();
	virtual size_t toSend();
	virtual size_t toSend(const int pid);
	virtual std::string collectMsg(const int pid);

	virtual std::string collectLocalProgress();

private:
	void intializedProcessCB(); // cache-based
	void intializedProcessACF(); // cache-free accumulative
	void intializedProcessSCF(); // cache-free selective

	void processNode(const id_t id); // need_commit() check before spread
	void processNodeForced(const id_t id); // no need_commit() check, directly spread
	void processNode_general(const id_t id);
	void processNode_acf(const id_t id);

private:
	int get_part(const id_t id){
		return ptn->owner(id);
	};
	bool is_local_part(const int pid){ return pid == local_id; }
	bool is_local_id(const id_t id){ return get_part(id) == local_id; }
	node_t& get_node(const id_t id){ return local_part.get(id); }

	void add_local_node(id_t& id, neighbor_list_t& nl);

	void update_cal(const id_t& from, const id_t& to, const value_t& v);
	void prepare_cal(const id_t& from, const id_t& to, const value_t& v);
	bool is_acf(){
		return cache_free && opt->is_accumulative();
	}

private:
	operation_t* opt;
	iohandler_t* ioh;
	SchedulerBase* scd;
	PartitionerBase* ptn;
	progressor_t* prg;
	size_t nPart;

	bool aggregate_message;
	bool incremental;
	bool async;
	bool cache_free;
	bool sort_result;
	
	std::vector<RemoteHolder<V, N>> remote_parts;
	LocalHolder<V, N> local_part;
	int local_id;

	//std::function<void(const id_t)> pf_processNode;
	using pf_pn_t = void (GlobalHolder<V, N>::*)(const id_t);
	pf_pn_t pf_processNode; // do not need to make related functions public

 	// used for incremental case of accumulative operators, store the source delta nodes
	// write at loadDelta(), clear at intializedProcess()
	std::unordered_map<id_t, node_t> touched_node;

	// buffer for in-neighbor of nodes on other workers
	std::vector<std::unordered_map<id_t, neighbor_list_t>> buf_in; // (node, in-list) for each worker
};

template <class V, class N>
void GlobalHolder<V, N>::init(OperationBase* opt, IOHandlerBase* ioh,
		SchedulerBase* scd, PartitionerBase* ptn, ProgressorBase* prg,
		const size_t nPart, const int localId, const bool aggregate_message,
		const bool incremental, const bool async, const bool cache_free, const bool sort_result)
{
	this->opt = dynamic_cast<operation_t*>(opt);
	this->ioh = dynamic_cast<iohandler_t*>(ioh);
	this->scd = scd;
	this->ptn = ptn;
	this->prg = dynamic_cast<progressor_t*>(prg);
	this->nPart = nPart;
	this->local_id = localId;
	this->aggregate_message = aggregate_message;
	this->incremental = incremental;
	this->async = async;
	this->cache_free = cache_free;
	this->sort_result = sort_result;

	this->ptn->setParts(nPart);

	local_part.init(this->opt, this->scd, this->prg, nPart, incremental, async, cache_free);
	remote_parts.resize(nPart);
	for(size_t i = 0; i<nPart; ++i){
		if(i == local_id)
			continue;
		remote_parts[i].init(this->opt, local_id, aggregate_message, incremental, cache_free);
	}

	if(is_acf()){
		// std::bind(&GlobalHolder::processNode_acf, this, std::placeholders::_1);
		pf_processNode = &GlobalHolder<V, N>::processNode_acf;
	}else{
		// std::bind(&GlobalHolder::processNode_general, this, std::placeholders::_1);
		pf_processNode = &GlobalHolder<V, N>::processNode_general;
	}
}

template<class V, class N>
inline size_t GlobalHolder<V, N>::numLocalNode()
{
	return local_part.size();
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
	local_part.init_value(d.first, d.second);
	return pid;
}

template <class V, class N>
int GlobalHolder<V, N>::loadDelta(const std::string& line){
	ChangeEdge<N> d = ioh->load_change(line);
	int pid = get_part(d.src);
	if(!is_local_part(pid))
		return pid;
	if(incremental){
		if(touched_node.find(d.src) == touched_node.end()){
			touched_node[d.src] = local_part.get(d.src);
		}
	}
	if(d.type == ChangeEdgeType::ADD){
		local_part.modify_onb_add(d.src, d.dst);
	}else if(d.type == ChangeEdgeType::REMOVE){
		// two steps: remove out-neighbor on source (local), remove cache on destination (may be remote)
		local_part.modify_onb_rmv(d.src, d.dst);
		id_t kd = get_key(d.dst);
		// reuse pid
		pid = get_part(kd);
		if(is_local_part(pid))
			local_part.modify_cache_rmv(d.src, kd);
		else
			return pid;
	}else{
		local_part.modify_onb_val(d.src, d.dst);
	}
	return pid;
}

template <class V, class N>
void GlobalHolder<V, N>::intializedProcess(){
	if(!cache_free){
		intializedProcessCB();
	}else{
		if(opt->is_accumulative()){
			intializedProcessACF();
		}else if(opt->is_selective()){
			intializedProcessSCF();
		}
	}
	touched_node.clear();
}

template <class V, class N>
void GlobalHolder<V, N>::intializedProcessCB(){
	if(incremental){
		for(const std::pair<id_t, node_t>& n : touched_node){
			processNodeForced(n.first);
		}
	}
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true);
		p != nullptr; p = local_part.enum_next(true))
	{
		if(incremental)
			local_part.cal_general(p->id); // batch update
		processNode(p->id);
	}
}
template <class V, class N>
void GlobalHolder<V, N>::intializedProcessACF(){
	// step 1: if loaded from existed result, move the value of dummy nodes from <u> to <v>
	if(incremental){
		std::vector<typename operation_t::DummyNode> dummies = opt->dummy_nodes();
		for(auto& p : dummies){
			id_t id = p.node.id;
			node_t& n = local_part.get(id);
			n.v = n.u;
			n.u = opt->identity_element();
		}
	}
	// step 2: generated initial messsages for changed nodes (incremental only)
	for(const std::pair<id_t, node_t>& n : touched_node){
		std::vector<std::pair<id_t, value_t>> old_d = opt->func(n.second);
		std::map<id_t, value_t> old_dm(old_d.begin(), old_d.end());
		old_d.clear();
		std::vector<std::pair<id_t, value_t>> new_d = opt->func(local_part.get(n.first));
		// update n.u of the changed values.
		for(const auto& p : new_d){
			auto it = old_dm.find(p.first);
			if(it == old_dm.end()){ // add an edge
				update_cal(n.first, p.first, p.second);
			}else{ // modify an edge
				if(p.second != it->second)
					update_cal(n.first, p.first, opt->ominus(p.second, it->second));
				old_dm.erase(it);
			}
		}
		for(const auto& p : old_dm){ // delete an edge
			update_cal(n.first, p.first, opt->ominus(opt->identity_element(), p.second));
		}
	}
}
template <class V, class N>
void GlobalHolder<V, N>::intializedProcessSCF(){
	for(const std::pair<id_t, node_t>& n : touched_node){
		std::vector<std::pair<id_t, value_t>> old_d = opt->func(n.second);
		std::map<id_t, value_t> old_dm(old_d.begin(), old_d.end());
		old_d.clear();
		std::vector<std::pair<id_t, value_t>> new_d = opt->func(local_part.get(n.first));
		// update n.u of the changed values.
		for(const auto& p : new_d){
			auto it = old_dm.find(p.first);
			if(it == old_dm.end()){ // add an edge
				update_cal(n.first, p.first, p.second);
			}else{ // modify an edge
				if(p.second != it->second)
					update_cal(n.first, p.first, p.second);
				old_dm.erase(it);
			}
		}
		for(const auto& p : old_dm){ // delete an edge
			update_cal(n.first, p.first, opt->identity_element());
		}
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
void GlobalHolder<V, N>::prepareCollectINList(){
	buf_in.resize(nPart);
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true);
		p != nullptr; p = local_part.enum_next(true))
	{
		for(auto& nb : p->onb){
			id_t dst = get_key(nb);
			int pid = get_part(dst);
			neighbor_t nei = make_in_neighbor(p->id, nb);
			if(is_local_part(pid)){
				local_part.add_in_neighbor(dst, nei);
			} else{
				buf_in[pid][dst].push_back(std::move(nei));
			}
		}
	}
}
template <class V, class N>
void GlobalHolder<V, N>::prepareCollectINCache(){
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true);
		p != nullptr; p = local_part.enum_next(true))
	{
		for(auto& nb : p->onb){
			id_t dst = get_key(nb);
			int pid = get_part(dst);
			value_t v = opt->func(*p, nb);
			if(is_local_part(pid)){
				local_part.update_cache(p->id, dst, v);
			}else{
				remote_parts[pid].update(p->id, dst, v);
			}
		}
	}
}
template <class V, class N>
void GlobalHolder<V, N>::rebuildSource(){
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true);
		p != nullptr; p = local_part.enum_next(true))
	{
		local_part.cal_general(p->id);
	}
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
void GlobalHolder<V, N>::clearINList(){
	buf_in.clear();
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true);
		p != nullptr; p = local_part.enum_next(true))
	{
		node_t* pp = const_cast<node_t*>(p);
		pp->inb.clear();
	}
}
template <class V, class N>
void GlobalHolder<V, N>::takeINList(const std::string& line){
	auto ms = deserialize<typename msg_t::MsgGINList_t>(line);
	for(typename msg_t::GINList_t& m : ms) {
		local_part.add_in_neighbor(m.first, m.second);
	}
}
template <class V, class N>
std::unordered_map<int, std::string> GlobalHolder<V, N>::collectINList(){
	std::unordered_map<int, std::string> res;
	res.reserve(nPart);
	for(int i = 0; i < static_cast<int>(nPart); ++i){
		if(is_local_part(i))
			continue;
		res[i] = serialize(buf_in[i]);
		buf_in[i].clear();
	}
	buf_in.clear();
	return res;
}

template <class V, class N>
void GlobalHolder<V, N>::clearINCache(){
	local_part.enum_rewind();
	for(const node_t* p = local_part.enum_next(true);
		p != nullptr; p = local_part.enum_next(true))
	{
		node_t* pp = const_cast<node_t*>(p);
		pp->cs.clear();
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
	std::unordered_map<int, std::string> res;
	res.reserve(nPart);
	for(int i = 0; i< static_cast<int>(nPart); ++i){
		if(is_local_part(i))
			continue;
		res[i] = collectINCache(i);
	}
	return res;
}
template <class V, class N>
std::string GlobalHolder<V, N>::collectINCache(const size_t pid){
	auto temp = remote_parts[pid].collect();
	return serialize(temp);
}

template <class V, class N>
void GlobalHolder<V, N>::msgUpdate(const std::string& line){
	auto ms = deserialize<typename msg_t::MsgVUpdate_t>(line);
//	DVLOG(3)<<"receive: "<<ms;
	for(typename msg_t::VUpdate_t& m : ms) {
		local_part.cal_incremental(std::get<0>(m), std::get<1>(m), std::get<2>(m));
	}
}
template <class V, class N>
std::string GlobalHolder<V, N>::msgRequest(const std::string& line){
	auto m = deserialize<typename msg_t::VRequest_t>(line);
	DVLOG(3)<<"receive request: "<<m;
	value_t v = local_part.get(m.second).v;
	return serialize(typename msg_t::VReply_t{m.first, m.second, v});
}
template <class V, class N>
void GlobalHolder<V, N>::msgReply(const std::string& line){
	auto m = deserialize<typename msg_t::VReply_t>(line);
	//DVLOG(3)<<"receive reply: "<<m;
	local_part.cal_incremental(std::get<1>(m), std::get<0>(m), std::get<2>(m));
}

template <class V, class N>
void GlobalHolder<V, N>::update_cal(const id_t& from, const id_t& to, const value_t& v){
	int pid = get_part(to);
	if(is_local_part(pid)){
		local_part.cal_incremental(from, to, v);
	}else{
		remote_parts[pid].update(from, to, v);
	}
}
template <class V, class N>
void GlobalHolder<V, N>::prepare_cal(const id_t& from, const id_t& to, const value_t& v){
	int pid = get_part(to);
	if(is_local_part(pid)){
		local_part.cal_prepare(from, to, v);
	}else{
		remote_parts[pid].prepare(from, to, v);
	}
}

template <class V, class N>
void GlobalHolder<V, N>::processNodeForced(const id_t id){
	#if !defined(NDEBUG) || defined(_DEBUG)
	if(VLOG_IS_ON(3)){
		const node_t& n = local_part.get(id);
		DVLOG(3) << "k=" << n.id << " v=" << n.v << " u=" << n.u << " cache=" << n.cs;
		auto pgs = local_part.get_progress();
		DVLOG(3) << "progress=(" << pgs.sum << "," << pgs.n_inf << "," << pgs.n_change << ") update=" << local_part.get_n_update();
	}
	#endif

	(this->*pf_processNode)(id);
}
template <class V, class N>
void GlobalHolder<V, N>::processNode(const id_t id){
	#if !defined(NDEBUG) || defined(_DEBUG)
	if(VLOG_IS_ON(3)){
		const node_t& n = local_part.get(id);
		DVLOG(3) << "k=" << n.id << " v=" << n.v << " u=" << n.u << " cache=" << n.cs;
		auto pgs = local_part.get_progress();
		DVLOG(3) << "progress=(" << pgs.sum << "," << pgs.n_inf << "," << pgs.n_change << ") update=" << local_part.get_n_update();
	}
	#endif

	if(!local_part.need_commit(id))
		return;
	//pf_processNode(id);
	(this->*pf_processNode)(id);
}
template <class V, class N>
void GlobalHolder<V, N>::processNode_general(const id_t id){
	// (commit -> spread)
	local_part.commit(id);
	std::vector<std::pair<id_t, value_t>> data = local_part.spread(id);
	DVLOG(3)<<data;
	for(auto& p : data){
		update_cal(id, p.first, p.second);
	}
}
template <class V, class N>
void GlobalHolder<V, N>::processNode_acf(const id_t id){
	// (spread -> commit)
	// prevent to merge self-loop
	std::vector<std::pair<id_t, value_t>> data = local_part.spread(id);
	DVLOG(3)<<data;
	V left = opt->identity_element();
	for(auto& p : data){
		if(p.first == id){
			left = p.second;
		}else{
			update_cal(id, p.first, p.second);
		}
	}
	local_part.commit(id);
	if(left != opt->identity_element())
		update_cal(id, id, left);
}

template <class V, class N>
size_t GlobalHolder<V, N>::toApply(){
	return !scd->empty();
}
template <class V, class N>
void GlobalHolder<V, N>::doApply(){
	std::vector<id_t> nodes = scd->pick();
	for(const id_t id : nodes){
		processNode(id);
	}
	#if !defined(NDEBUG) || defined(_DEBUG)
	if(VLOG_IS_ON(3)){
		for(const id_t id : nodes){
			const node_t& n = local_part.get(id);
			DVLOG(3) << "k=" << n.id << " v=" << n.v << " u=" << n.u;
		}
	}
	#endif
}

template <class V, class N>
size_t GlobalHolder<V, N>::toSend(){
	size_t sum = 0;
	for(auto& t : remote_parts){
		sum += t.size();
	}
	return sum;
}
template <class V, class N>
size_t GlobalHolder<V, N>::toSend(const int pid){
	return remote_parts[pid].size();
}

template <class V, class N>
std::string GlobalHolder<V, N>::collectMsg(const int pid){
	// msg_t::MsgVUpdate_t = std::vector<typename msg_t::VUpdate_t>
	// std::vector<std::pair<id_t, std::pair<id_t, value_t>>> data =
	typename msg_t::MsgVUpdate_t data =
		remote_parts[pid].collect();
//	DVLOG(3)<<"send: "<<data;
	std::string res = serialize(data);
	return res;
}

template <class V, class N>
std::string GlobalHolder<V, N>::collectLocalProgress(){
	ProgressReport progress = local_part.get_progress();
	local_part.reset_progress_count();
	return serialize(progress);
}
