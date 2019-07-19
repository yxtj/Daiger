#include "Scheduler.h"
#include <numeric>
#include <unordered_map>
#include <unordered_set>
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

void SchedulerRoundRobin::init(const std::vector<std::string>& args){
	nNode = 0;
	loop_pointer = 0;
	try{
		if(args.size() > 1)
			portion = stod(args[1]);
		else
			portion = 1.0;
	} catch(exception& e){
		throw invalid_argument("Unable to get <portion> for SchedulerRoundRobin.");
	}
}
void SchedulerRoundRobin::regist(const id_t& k){
	SchedulerBase::regist(k);
	data.push_back(k);
}
void SchedulerRoundRobin::ready(){
	n_each_pick = static_cast<size_t>(ceil(portion * nNode));
	n_each_pick = max<size_t>(1, n_each_pick);
	n_each_pick = min<size_t>(nNode, n_each_pick);
}

bool SchedulerRoundRobin::empty() const {
	return data.empty();
}
size_t SchedulerRoundRobin::size() const
{
	return nNode;
}
void SchedulerRoundRobin::update(const id_t& k, const priority_t& p){
}
id_t SchedulerRoundRobin::top(){
	return data[loop_pointer];
}
void SchedulerRoundRobin::pop(){
	loop_pointer = (loop_pointer + 1) % nNode;
}
std::vector<id_t> SchedulerRoundRobin::pick_n(const size_t n){
	std::vector<id_t> res;
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
id_t SchedulerRoundRobin::pick_one(){
	id_t k = data[loop_pointer];
	loop_pointer = (loop_pointer + 1) % nNode;
	return k;
}
std::vector<id_t> SchedulerRoundRobin::pick(){
	//return data;
	return pick_n(n_each_pick);
}

// -------- prdefined class SchedulerPriorityMaintain with SCH_PrioritizedMaintainHolder --------

struct SCH_PrioritizedMaintainHolder{
	//SCH_PrioritizedMaintainHolder();

	id_t top();
	void pop();

	void update(const id_t& k, const priority_t& p);
	void reset(const id_t& k);

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
	unordered_map<id_t, handle_t> khm; // key-handler mapper
};

//SCH_PrioritizedMaintainHolder::SCH_PrioritizedMaintainHolder()
//	: heap(CmpUnit())
//{}
id_t SCH_PrioritizedMaintainHolder::top(){
	return heap.top().k;
}
void SCH_PrioritizedMaintainHolder::pop(){
	id_t k = top();
	heap.pop();
	khm.erase(k);
}
void SCH_PrioritizedMaintainHolder::update(const id_t& k, const priority_t& p){
	auto it = khm.find(k);
	if(it == khm.end()){ // new key -> push
		khm[k] = heap.push(Unit{k, p});
	}else{ // exist key -> update
		heap.update(it->second, Unit{k, p});
	}
}
void SCH_PrioritizedMaintainHolder::reset(const id_t& k){
	auto it = khm.find(k);
	if(it != khm.end()){ 
		heap.erase(it->second);
		khm.erase(it);
	}
}

SchedulerPriorityMaintain::SchedulerPriorityMaintain()
	: data(nullptr)
{}
SchedulerPriorityMaintain::~SchedulerPriorityMaintain(){
	delete data;
}
void SchedulerPriorityMaintain::init(const std::vector<std::string>& args){
	nNode = 0;
	data = new SCH_PrioritizedMaintainHolder();
	try{
		portion = stod(args[1]);
	} catch(exception& e){
		throw invalid_argument("Unable to get <portion> for SchedulerPriorityMaintain.");
	}
}
void SchedulerPriorityMaintain::ready(){
	n_each_pick = static_cast<size_t>(ceil(portion * nNode));
	n_each_pick = max<size_t>(1, n_each_pick);
}

bool SchedulerPriorityMaintain::empty() const {
	return data->empty();
}
size_t SchedulerPriorityMaintain::size() const
{
	return data->size();
}
void SchedulerPriorityMaintain::update(const id_t& k, const priority_t& p){
	data->update(k, p);
}
id_t SchedulerPriorityMaintain::top(){
	return data->top();
}
void SchedulerPriorityMaintain::pop(){
	data->pop();
}
std::vector<id_t> SchedulerPriorityMaintain::pick_n(const size_t n){
	std::vector<id_t> res;
	size_t i=0;
	while(!data->empty() && i++ < n){
		res.push_back(data->top());
		data->pop();
	}
	return res;
}
id_t SchedulerPriorityMaintain::pick_one(){
	id_t k=data->top();
	data->pop();
	return k;
}
std::vector<id_t> SchedulerPriorityMaintain::pick(){
	return pick_n(n_each_pick);
}

// -------- prdefined class SchedulerPrioritySelection with SCH_PrioritizedSelectionHolder --------

struct SCH_PrioritizedSelectionHolder{
	id_t top();
	void pop();
	vector<id_t> pickTops(const size_t k);

	void update(const id_t& k, const priority_t& p);

	size_t size() const {
		return priority.size();
	}
	bool empty() const {
		return priority.empty();
	}
	void clear() {
		priority.clear();
		index.clear();
	}
	void reserve(const size_t n) {
		priority.reserve(n);
		index.reserve(n);
	}

private:
	void prepare(const size_t k); // put the highest at the bottom

	using Unit = SchedulerBase::Unit;
	using CmpUnit = SchedulerBase::CmpUnit;
	unordered_map<id_t, priority_t> priority;
	vector<id_t> index;
};

id_t SCH_PrioritizedSelectionHolder::top(){
	prepare(1);
	return index.back();
}
void SCH_PrioritizedSelectionHolder::pop(){
	id_t k = index.back();
	priority.erase(k);
	index.pop_back();
}
vector<id_t> SCH_PrioritizedSelectionHolder::pickTops(const size_t k)
{
	prepare(k);
	vector<id_t> res;
	size_t start;
	if(size() > k)
		start = size() - k;
	else
		start = 0;
	for(size_t i = start; i < index.size(); ++i){
		id_t k = index[i];
		res.push_back(k);
		priority.erase(k);
	}
	index.erase(index.begin() + start, index.end());
	return res;
}
void SCH_PrioritizedSelectionHolder::update(const id_t& k, const priority_t& p){
	auto it = priority.find(k);
	if(it == priority.end()){
		priority[k] = p;
		index.push_back(k);
	} else{
		it->second = p;
	}
}
void SCH_PrioritizedSelectionHolder::prepare(const size_t k)
{
	if(size() <= k)
		return;
	auto it = index.rbegin() + k;
	nth_element(index.rbegin(), it, index.rend(),
		[&](const id_t& l, const id_t& r){
		return priority[l] > priority[r];
	});
}

SchedulerPrioritySelection::SchedulerPrioritySelection()
	: data(nullptr)
{}
SchedulerPrioritySelection::~SchedulerPrioritySelection(){
	delete data;
}
void SchedulerPrioritySelection::init(const std::vector<std::string>& args){
	nNode = 0;
	data = new SCH_PrioritizedSelectionHolder();
	try{
		portion = stod(args[1]);
	} catch(exception& e){
		throw invalid_argument("Unable to get <portion> for SchedulerPrioritySelection.");
	}
}
void SchedulerPrioritySelection::ready(){
	n_each_pick = static_cast<size_t>(ceil(portion * nNode));
	n_each_pick = max<size_t>(1, n_each_pick);

	data->reserve(nNode);
}

bool SchedulerPrioritySelection::empty() const {
	return data->empty();
}
size_t SchedulerPrioritySelection::size() const {
	return data->size();
}
void SchedulerPrioritySelection::update(const id_t& k, const priority_t& p){
	data->update(k, p);
}
id_t SchedulerPrioritySelection::top(){
	return data->top();
}
void SchedulerPrioritySelection::pop(){
	data->pop();
}
std::vector<id_t> SchedulerPrioritySelection::pick_n(const size_t n){
	return data->pickTops(n);
}
id_t SchedulerPrioritySelection::pick_one(){
	id_t k = data->top();
	data->pop();
	return k;
}
std::vector<id_t> SchedulerPrioritySelection::pick(){
	return pick_n(n_each_pick);
}

// -------- prdefined class SchedulerFIFO with SCH_FIFOHolder --------

struct SCH_FIFOHolder{
	id_t top();
	void pop();

	void update(const id_t& k, const priority_t& p);

	size_t size() const { return que.size(); }
	bool empty() const { return que.empty(); }
	void clear() { 
		used.clear();
	}
	void reserve(const size_t n) {
		used.reserve(n);
	}
	
	queue<id_t> que;
	unordered_map<id_t, bool> used;
};
id_t SCH_FIFOHolder::top(){
	return que.front();
}
void SCH_FIFOHolder::pop(){
	id_t k = que.front();
	que.pop();
	used[k]=false;
}
void SCH_FIFOHolder::update(const id_t& k, const priority_t& p){
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

void SchedulerFIFO::init(const std::vector<std::string>& args){
	nNode = 0;
	data = new SCH_FIFOHolder();
}
void SchedulerFIFO::ready(){
	data->reserve(nNode);
}

bool SchedulerFIFO::empty() const {
	return data->empty();
}
size_t SchedulerFIFO::size() const
{
	return data->size();
}
void SchedulerFIFO::update(const id_t& k, const priority_t& p){
	data->update(k, p);
}
id_t SchedulerFIFO::top(){
	return data->top();
}
void SchedulerFIFO::pop(){
	data->pop();
}
std::vector<id_t> SchedulerFIFO::pick_n(const size_t n){
	std::vector<id_t> res;
	size_t i=0;
	while(!data->empty() && ++i < n){
		res.push_back(data->top());
		data->pop();
	}
	return res;
}
id_t SchedulerFIFO::pick_one(){
	id_t k=data->top();
	data->pop();
	return k;
}
std::vector<id_t> SchedulerFIFO::pick(){
	return pick_n(data->size());
}

