#include "Scheduler.h"
#include <functional>  // std::greater
#include <unordered_map>
#include <boost/heap/fibonacci_heap.hpp>

using namespace std;


struct SchedulerBase::PrioritizedHolder{
	using heap_t = boost::heap::fibonacci_heap<priority_t, boost::heap::compare<greater<priority_t>> >;
	heap_t heap;
	unordered_map<key_t, typename heap_t::handle_type> key_mapper;
};

SchedulerBase::SchedulerBase()
	: data(nullptr)
{}

SchedulerBase::~SchedulerBase()
{
	delete data;
}

void SchedulerBase::parse(const size_t n_nodes, const std::vector<std::string>& arg){
	this->nNode=n_nodes;

}


