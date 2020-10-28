#include "SyncUnit.h"

void SyncUnit::wait() {
	if(ready)	return;
	std::unique_lock<std::mutex> ul(m);
	if(ready)	return;
	cv.wait(ul, [&]() {return ready; });
}

bool SyncUnit::wait_for(const double& dur) {
	return wait_for(std::chrono::duration<double>(dur));
}

void SyncUnit::wait_reset() {
	std::unique_lock<std::mutex> ul(m);
	cv.wait(ul, [&]() {return ready; }); // directly return if ready==true
	ready = false;
}

bool SyncUnit::wait_reset_for(const double dur) {
	std::unique_lock<std::mutex> ul(m);
	bool ret = cv.wait_for(ul, std::chrono::duration<double>(dur), [&]() {return ready; });
	ready = false;
	return ret;
}

void SyncUnit::notify() {
	ready = true;
	cv.notify_all();
}
