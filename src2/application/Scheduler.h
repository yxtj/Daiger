#include "common/def.h"
#include "common/Node.h"
#include <vector>

class SchedulerBase {
public:
	SchedulerBase();
	virtual ~SchedulerBase();

	virtual void parse(const size_t nnodes, const std::vector<std::string>& arg);
	virtual priority_t lowest() const { return 0; }
	virtual bool prioritized(const priority_t a, const priority_t b){ return a > b; }

	virtual void update_priority(const key_t& k, const priority_t& p) = 0;
	// get the node id with top priority
	virtual key_t top() = 0;
	// reset the top node's priority to 0
	virtual void reset_top() = 0;
	// get the top <n> nodes and reset their priorities to lowest
	virtual std::vector<key_t> pop_some(const size_t n) = 0;
	virtual key_t pop() = 0; // as a special version of pop_some(1);


protected:
	size_t nNode;
	struct PrioritizedHolder;
	PrioritizedHolder* data;
};

template <typename V, typename N>
class SchedulerBaseT
	: public SchedulerBase
{
public:
	// calculate the priority of a node
	virtual priority_t priority(const Node<V, N>& n) = 0;
};
