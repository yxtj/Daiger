#include "Partitioner.h"
#include <stdexcept>

using namespace std;

void PartitionerBase::setParts(const size_t n){
	nWorker = n;
}

// -------- PartitionerMod --------

size_t PartitionerMod::owner(const id_t& id) const {
	return id % nWorker;
}
