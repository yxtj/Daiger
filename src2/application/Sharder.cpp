#include "Sharder.h"
#include <stdexcept>

using namespace std;

void SharderBase::setParts(const size_t n){
	nWorker = n;
}

// -------- SharderMod --------

size_t SharderMod::owner(const id_t& id){
	return id % nWorker;
}
