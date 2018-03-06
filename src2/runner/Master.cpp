#include "Master.h"
#include "network/NetworkThread.h"
#include <functional>
#include <chrono>

using namespace std;

Master::Master(const AppBase& app, Option& opt)
	: Runner(app, opt)
{
}

void Master::start() {
	start_running();
    registerWorker();

    procedureLoadGraph();
    if (opt.do_incremental) {
        procedureLoadValue();
        procedureLoadDelta();
    }
    procedureUpdate();
    if (opt.do_output) {
        procedureOutput();
    }
}

void Master::finish() {
    terminateWorker();
	stop_running();
    tmsg.join();
}
