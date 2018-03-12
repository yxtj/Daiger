#include "def_func.h"

using namespace std;

id_t stoid(const std::string& str){
	return static_cast<id_t>(std::stoul(str));
}

priority_t stop(const std::string& str){
	return std::stof(str);
}
