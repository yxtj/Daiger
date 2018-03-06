#include "Worker.h"
#include "msg/MType.h"
#include "serial/serialization.h"
#include "runner_helpers.h"
#include <chrono>
#include <functional>
#include <string>
#include <thread>

using namespace std;

// register helpers
Worker::callback_t Worker::localBinder(void (Worker::*fp)(const std::string&, const RPCInfo&)){
	return bind(fp, this, placeholders::_1, placeholders::_2);
}

void Worker::registerHandlers() {
    // TODO: add handlers

}
