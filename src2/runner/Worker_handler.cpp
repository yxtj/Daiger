#include "Worker.h"
#include "msg/MType.h"
#include "network/NetworkThread.h"
#include "serial/serialization.h"
#include "runner_helpers.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>

using namespace std;

// register helpers
Worker::callback_t Worker::localCBBinder(
	void (Worker::*fp)(const std::string&, const RPCInfo&))
{
	return bind(fp, this, placeholders::_1, placeholders::_2);
}

void Worker::registerHandlers() {
	int nw=opt.nPart;
	ReplyHandler::ConditionType EACH_ONE=ReplyHandler::EACH_ONE;
	
	// part 1: message handler
	regDSPProcess(MType::CReady, localCBBinder(&Worker::handleReply));
	regDSPProcess(MType::CRegister, localCBBinder(&Worker::handleRegister));
	regDSPProcess(MType::CWorkers, localCBBinder(&Worker::handleWorkers));

	// part 2: reply handler:
	//type 1: called by handleReply() directly

	//type 2: called by specific functions (handlers)
	// by handlerRegisterWorker()
	// addRPHEachSU(MType::CRegister, &su_regw);
}

void Worker::handleReply(const std::string& d, const RPCInfo& info) {
    int type = deserialize<int>(d);
	int source = wm.nid2wid(info.source);
    rph.input(type, source);
}

void Worker::handleRegister(const std::string& d, const RPCInfo& info){
	int nid = deserialize<int>(d);
	master_net_id = nid;
	net->send(master_net_id, MType::CRegister, net->id());
}

void Worker::handleWorkers(const std::string& d, const RPCInfo& info){
	vector<pair<int, int>> idmapping;
	for(auto& p : idmapping){
		wm.register_worker(p.first, p.second);
	}
	sendReply(info);
}
