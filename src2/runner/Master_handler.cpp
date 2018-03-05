#include "Master.h"
#include "msg/MType.h"
#include "serial/serialization.h"
#include "IdTranslate.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>

using namespace std;
using namespace std::placeholders;

// register helpers
void Master::regDSPImmediate(const int type, callback_t fp) {
    driver.registerImmediateHandler(type, bind(fp, this, _1, _2));
}
void Master::regDSPProcess(const int type, callback_t fp) {
    driver.registerProcessHandler(type, bind(fp, this, _1, _2));
}
void Master::regDSPDefault(callback_t fp) {
    driver.registerDefaultOutHandler(bind(fp, this, _1, _2));
}
void Master::addReplyHandler(
    const int mtype, void (Master::*fp)(), const bool newThread) {
    rph.addType(mtype,
        ReplyHandler::condFactory(ReplyHandler::EACH_ONE, opt.nPart),
        bind(fp, this), newThread);
}

void Master::registerHandlers() {
    // TODO: add handlers
}

void Master::handleReply(const std::string& d, const RPCInfo& info) {
    msg_t type = deserialize<msg_t>(d);
    rph.input(type, nidtrans(info.source).second);
}
