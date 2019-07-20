#include "Partitioner.h"
#include <stdexcept>

using namespace std;

void PartitionerBase::setParts(const size_t n){
	nWorker = n;
}

// -------- PartitionerMod --------

int PartitionerMod::owner(const id_t& id) const {
	return static_cast<int>(id % nWorker);
}
