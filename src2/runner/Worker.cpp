#include "Worker.h"
#include "network/NetworkThread.h"
#include <functional>
#include <chrono>

using namespace std;

Worker::Worker(const AppBase& app, Option& opt)
	: Runner(app, opt)
{
}

void Worker::start() {
	start_running();
    registerWorker();

}

void Worker::finish() {
    terminateWorker();
	stop_running();
    tmsg.join();
}
