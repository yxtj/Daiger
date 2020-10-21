#pragma once
/**
 * This file is used to simplify the usage of logging modules.
 * If some other logger module is used, just change the include line.
 */
#ifdef USE_ELPP
#include "easylogging++.h"
#else
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#endif

#include <string>

void initLogger(int argc, char* argv[], bool color=false);
void setLogThreadName(const std::string& name);
