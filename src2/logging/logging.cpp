#include "logging.h"

#ifdef USE_ELPP

INITIALIZE_EASYLOGGINGPP

void initLogger(int argc, char* argv[], bool color){
	START_EASYLOGGINGPP(argc, argv);

	el::Configurations defaultConf;
	defaultConf.setToDefault();
	// Values are always std::string
	defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");
#ifndef NDEBUG
	defaultConf.setGlobally(el::ConfigurationType::Format,
		"%datetime{%H:%m:%s.%g} (%thread) %level %fbase:%line] %msg");
#else
	defaultConf.setGlobally(el::ConfigurationType::Format,
		"%datetime{%H:%m:%s.%g} (%thread) %level] %msg");
#endif
	defaultConf.set(el::Level::Debug,
		el::ConfigurationType::Format, "%datetime{%H:%m:%s.%g} (%thread) %level %fbase:%line] %msg");

	// default logger uses default configurations
	el::Loggers::reconfigureLogger("default", defaultConf);
	if(color)
		el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
}

void setLogThreadName(const std::string& name){
	el::Helpers::setThreadName(name);
}

#else
void initLogger(int argc, char* argv[], bool color){
	FLAGS_logtostderr = false;
	FLAGS_logbuflevel = -1;
	if(color)
		FLAGS_colorlogtostderr = true;

	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	google::InstallFailureSignalHandler();

}

void setLogThreadName(const std::string& name){
}
#endif
