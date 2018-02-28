#include "Sharder.h"
#include <stdexcept>

using namespace std;

void SharderMod::parse(const size_t nworkers, const std::vector<std::string>& arg){
	this->nworkers = nworkers;
}
size_t SharderMod::owner(const key_t& id){
	return id % nworkers;
}
