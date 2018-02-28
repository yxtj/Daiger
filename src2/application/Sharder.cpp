#include "Sharder.h"
#include <stdexcept>

using namespace std;

void SharderBase::parse(const size_t n_workers, const std::vector<std::string>& arg){
	this->nWorker = n_workers;
}

size_t SharderMod::owner(const key_t& id){
	return id % nWorker;
}
