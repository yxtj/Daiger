#include "common/def.h"
#include <vector>

class SchedulerBase {
public:
	virtual ~SchedulerBase() = default;
	// return the lowest priority value
	virtual priority_t lowest() const;
	// virtual bool prioritized(const priority_t a, const priority_t b){ return a > b; }

	virtual void init(const std::vector<std::string>& arg);
	// let the scheduler know each node
	virtual void regist(const key_t& k){ ++nNode; }
	// make the scheduler ready to run (call after all nodes are registed)
	virtual void ready() {}

	// update/set the priority of a node
	virtual void update(const key_t& k, const priority_t& p) = 0;
	// get the node id with top priority
	virtual key_t top() = 0;
	// reset the top node's priority to the lowest
	virtual void pop() = 0;
	// get the top <n> nodes and reset their priorities to the lowest
	virtual std::vector<key_t> pick_n(const size_t n) = 0;
	virtual key_t pick_one() = 0; // as a special version of pick_n(1);
	virtual std::vector<key_t> pick() = 0; // get the top group

public:
	struct Unit{
		key_t k;
		priority_t p;
	};
	struct CmpUnit{
		bool operator()(const Unit& a, const Unit& b){
			return a.p > b.p;
		}
	};
protected:
	size_t nNode;
};

// -------- predefined round-robin Scheduler --------

/**
 * Round-Robin on all the registed nodes, in their registing order
 */
class SchedulerRoundRobin
	: public SchedulerBase
{
public:
	virtual void init(const std::vector<std::string>& args);
	virtual void regist(const key_t& k);

	virtual void update(const key_t& k, const priority_t& p);
	virtual key_t top();
	virtual void pop();
	virtual std::vector<key_t> pick_n(const size_t n);
	virtual key_t pick_one();
	virtual std::vector<key_t> pick();

private:
	size_t loop_pointer;
	std::vector<key_t> data;
};

// -------- predefined priority-based Scheduler --------

struct SCH_PrioritizedHolder;
/**
 * Pick the top <portion>*<nNode> nodes in terms of their priority
 */
class SchedulerPriority
	: public SchedulerBase
{
public:
	SchedulerPriority();
	virtual ~SchedulerPriority();

	virtual void init(const std::vector<std::string>& args);
	virtual void ready();

	virtual void update(const key_t& k, const priority_t& p);
	virtual key_t top();
	virtual void pop();
	virtual std::vector<key_t> pick_n(const size_t n);
	virtual key_t pick_one();
	virtual std::vector<key_t> pick();

private:
	double portion;
	size_t n_each_pick;
	SCH_PrioritizedHolder* data;
};

// -------- predefined FIFO Scheduler --------

struct SCH_FIFOHolder;
/**
 * FIFO of touched nodes
 */
class SchedulerFIFO
	: public SchedulerBase
{
public:
	SchedulerFIFO();
	virtual ~SchedulerFIFO();

	virtual void init(const std::vector<std::string>& args);
	virtual void regist(const key_t& k);
	virtual void ready();

	virtual void update(const key_t& k, const priority_t& p);
	virtual key_t top();
	virtual void pop();
	virtual std::vector<key_t> pick_n(const size_t n);
	virtual key_t pick_one();
	virtual std::vector<key_t> pick();

private:
	SCH_FIFOHolder* data;
};
