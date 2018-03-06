#include "Master.h"
#include "msg/MType.h"
#include "serial/serialization.h"
#include "runner_helpers.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>

using namespace std;

// register helpers
Master::callback_t Master::localBinder(void (Master::*fp)(const std::string&, const RPCInfo&)){
	return bind(fp, this, placeholders::_1, placeholders::_2);
}

void Master::registerHandlers() {
    // TODO: add handlers

}
