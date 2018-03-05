#include "Sharder.h"
#include <stdexcept>

using namespace std;

void SharderBase::init(const std::vector<std::string>& args){
	try{
		this->nWorker = stoi(args[0]);
	} catch(exception& e){
		throw invalid_argument("Unable to get <nWorker> for SharderBase");
	}
}

size_t SharderMod::owner(const id_t& id){
	return id % nWorker;
}
