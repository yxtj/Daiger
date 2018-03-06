#include "Timer.h"

using namespace std;

std::chrono::system_clock::time_point Timer::_boot_time = chrono::system_clock::now();

Timer::Timer()
{
	restart();
}

void Timer::restart()
{
	_time = chrono::system_clock::now();
}

long long Timer::elapseMS() const
{
	return chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now() - _time).count();
}

long long Timer::elapseS() const
{
	return chrono::duration_cast<chrono::seconds>(
		chrono::system_clock::now() - _time).count();
}

double Timer::elapseMin() const
{
	std::chrono::duration<double, ratio<60> > passed = chrono::system_clock::now() - _time;
	return passed.count();
}

double Timer::Now(){
	return std::chrono::duration<double>(
		chrono::system_clock::now().time_since_epoch()).count();
}

double NowSinceBoot(){
	return std::chrono::duration_cast<double>(
		chrono::system_clock::now() - _boot_time).count();
}