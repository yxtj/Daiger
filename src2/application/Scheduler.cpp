#include "Scheduler.h"
#include <numeric>
#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>
//#include <boost/heap/binomal_heap.hpp>

using namespace std;

// -------- base class --------

void SchedulerBase::init(const std::vector<std::string>& arg)
{
	nNode = 0;
}

priority_t SchedulerBase::lowest() const{
	return numeric_limits<priority_t>::lowest();
}

// -------- predefined class SchedulerRoundRobin --------

void SchedulerRoundRobin::init(const std::vector<std::string>& arg){
	loop_pointer = 0;
}
void SchedulerRoundRobin::regist(const key_t& k){
	SchedulerBase::regist(k);
	data.push_back(k);
}

void SchedulerRoundRobin::update(const key_t& k, const priority_t& p){
}
key_t SchedulerRoundRobin::top(){
	return data[loop_pointer];
}
void SchedulerRoundRobin::pop(){
	loop_pointer = (loop_pointer + 1) % nNode;
}
std::vector<key_t> SchedulerRoundRobin::pick_n(const size_t n){
	std::vector<key_t> res;
	if(n==0)
		return res;
	size_t num = n<=nNode?n:nNode;
	res.reserve(num);
	auto end = loop_pointer+num<nNode ? data.begin()+loop_pointer+num : data.end();
	for(auto it = data.begin()+loop_pointer; it!=end; ++it)
		res.push_back(*it);
	if(loop_pointer+num > nNode){
		end=data.begin()+(num - (nNode-loop_pointer));
		for(auto it = data.begin(); it!=end; ++it)
			res.push_back(*it);
	}
	loop_pointer = (loop_pointer+num) % nNode;
	return res;
}
key_t SchedulerRoundRobin::pick_one(){
	key_t k = data[loop_pointer];
	loop_pointer = (loop_pointer + 1) % nNode;
	return k;
}
std::vector<key_t> SchedulerRoundRobin::pick(){
	return data;
}

// -------- prdefined class SchedulerPriority with SCH_PrioritizedHolder --------

struct SCH_PrioritizedHolder{
	//SCH_PrioritizedHolder();

	key_t top();
	void pop();

	void update(const key_t& k, const priority_t& p);
	void reset(const key_t& k);

	size_t size() const { return heap.size(); }
	bool empty() const { return heap.empty(); }
	void clear() { 
		heap.clear();
		khm.clear();
	}
	void reserve(const size_t n) {
		khm.reserve(n);
	}

private:
	using Unit = SchedulerBase::Unit;
	using CmpUnit = SchedulerBase::CmpUnit;
	using heap_t = boost::heap::fibonacci_heap<Unit, boost::heap::compare<CmpUnit> >;
	//using heap_t = boost::heap::binomal_heap<Unit, boost::heap::compare<CmpUnit> >;
	using handle_t = typename heap_t::handle_type;
	heap_t heap;
	unordered_map<key_t, handle_t> khm; // key-handler mapper
};

//SCH_PrioritizedHolder::SCH_PrioritizedHolder()
//	: heap(CmpUnit())
//{}
key_t SCH_PrioritizedHolder::top(){
	return heap.top().k;
}
void SCH_PrioritizedHolder::pop(){
	key_t k = top();
	heap.pop();
	khm.erase(k);
}
void SCH_PrioritizedHolder::update(const key_t& k, const priority_t& p){
	auto it = khm.find(k);
	if(it == khm.end()){ // new key -> push
		khm[k] = heap.push(Unit{k, p});
	}else{ // exist key -> update
		heap.update(it->second, Unit{k, p});
	}
}
void SCH_PrioritizedHolder::reset(const key_t& k){
	auto it = khm.find(k);
	if(it != khm.end()){ 
		heap.erase(it->second);
		khm.erase(it);
	}
}

SchedulerPriority::SchedulerPriority()
	: data(nullptr)
{}
SchedulerPriority::~SchedulerPriority(){
	delete data;
}
void SchedulerPriority::init(const std::vector<std::string>& arg){
	data = new SCH_PrioritizedHolder();
	try{
		portion = stod(arg[0]);
	} catch(exception& e){
		throw invalid_argument("Unable to get <portion> for SchedulerPriority.");
	}
}
void SchedulerPriority::ready(){
	n_each_pick = static_cast<size_t>(ceil(portion * nNode));
}

void SchedulerPriority::update(const key_t& k, const priority_t& p){
	data->update(k, p);
}
key_t SchedulerPriority::top(){
	return data->top();
}
void SchedulerPriority::pop(){
	data->pop();
}
std::vector<key_t> SchedulerPriority::pick_n(const size_t n){
	std::vector<key_t> res;
	size_t i=0;
	while(!data->empty() && ++i < n){
		res.push_back(data->top());
		data->pop();
	}
	return res;
}
key_t SchedulerPriority::pick_one(){
	key_t k=data->top();
	data->pop();
	return k;
}
std::vector<key_t> SchedulerPriority::pick(){
	return pick_n(n_each_pick);
}

// -------- prdefined class SchedulerFIFO with SCH_FIFOHolder --------

struct SCH_FIFOHolder{
	key_t top();
	void pop();

	void update(const key_t& k, const priority_t& p);

	size_t size() const { return que.size(); }
	bool empty() const { return que.empty(); }
	void clear() { 
		used.clear();
	}
	void reserve(const size_t n) {
		used.reserve(n);
	}
	
	queue<key_t> que;
	unordered_map<key_t, bool> used;
};
key_t SCH_FIFOHolder::top(){
	return que.front();
}
void SCH_FIFOHolder::pop(){
	key_t k = que.front();
	que.pop();
	used[k]=false;
}
void SCH_FIFOHolder::update(const key_t& k, const priority_t& p){
	if(used[k] == false){ // new <k> is guaranteed to have a value false
		que.push(k);
		used[k]=true;
	}
}

SchedulerFIFO::SchedulerFIFO()
	: data(nullptr)
{}
SchedulerFIFO::~SchedulerFIFO(){
	delete data;
}

void SchedulerFIFO::init(const std::vector<std::string>& arg){
	data = new SCH_FIFOHolder();
}
void SchedulerFIFO::ready(){
	data->reserve(nNode);
}

void SchedulerFIFO::update(const key_t& k, const priority_t& p){
	data->update(k, p);
}
key_t SchedulerFIFO::top(){
	return data->top();
}
void SchedulerFIFO::pop(){
	data->pop();
}
std::vector<key_t> SchedulerFIFO::pick_n(const size_t n){
	std::vector<key_t> res;
	size_t i=0;
	while(!data->empty() && ++i < n){
		res.push_back(data->top());
		data->pop();
	}
	return res;
}
key_t SchedulerFIFO::pick_one(){
	key_t k=data->top();
	data->pop();
	return k;
}
std::vector<key_t> SchedulerFIFO::pick(){
	return pick_n(data->size());
}

