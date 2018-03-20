#include "logger.h"

using namespace std;

INITIALIZE_EASYLOGGINGPP

void initLogger(int argc, char* argv[]){
	START_EASYLOGGINGPP(argc, argv)
	
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	// Values are always std::string
	defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");
	defaultConf.setGlobally(el::ConfigurationType::Format,
		"%datetime{%H:%m:%s.%g} %level (%thread_name) %msg");
	defaultConf.set(el::Level::Debug, 
		el::ConfigurationType::Format, "%datetime{%H:%m:%s.%g} %level (%thread_name) %loc] %msg");
			
	// default logger uses default configurations
	el::Loggers::reconfigureLogger("default", defaultConf);
}

void setLocalThreadName(const std::string& name){
	el::Helpers::setThreadName(name);
}
