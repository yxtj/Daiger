#include "mc.h"
#include "util/Util.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <random>
#include <functional>

using namespace std;

const std::string MarkovChain::name("mc");

std::string MarkovChain::getName() const {
	return name;
}

void MarkovChain::reg(){
	AppKernelFactory::registerClass<MarkovChain>(name);
}

ArgumentSeparator* MarkovChain::generateSeparator(){
	return new separator_t();
}
OperationBase* MarkovChain::generateOperation(){
	return new operation_t();
}
IOHandlerBase* MarkovChain::generateIOHandler(){
	return new iohandler_t();
}
ProgressorBase* MarkovChain::generateProgressor(){
	return new progressor_t();
}
GlobalHolderBase* MarkovChain::generateGraph(){
	return new graph_t();
}

// -------- Components --------

struct MarkovChain::MyOperation::InnerHelper{
	mt19937 gen;
	uniform_real_distribution<double> dis;
	function<double()> fun;

	bool init(const string& im);
};

bool MarkovChain::MyOperation::InnerHelper::init(const string& im){
	size_t p = im.find(':');
	if(p==string::npos)
		return false;
	string m=im.substr(0, p);
	if(p == 3 && m == "fix"){
		try{
			double v = stod(im.substr(p+1));
			fun = [=](){
				return v;
			};
		}catch(...){
			return false;
		}
	}else if(p == 4 && m == "rand"){
		try{
			unsigned long s = stoul(im.substr(p+1));
			gen.seed(s);
			dis = uniform_real_distribution<double>(0.0, 1.0);
			fun = [&](){
				return dis(gen);
			};
		}catch(...){
			return false;
		}
	}else{
		return false;
	}
	return true;
}

void MarkovChain::MyOperation::init(const std::vector<std::string>& arg_line, const size_t nInstance){
	impl = new InnerHelper();
	if(!impl->init(arg_line[0]))
		throw invalid_argument("cannot parse the value initialization method. Support: fix:<v> and rand:<s>");
	// TODO: add resource cleaning interface to Operation

	use_degree = beTrueOption(arg_line[1]);
}
MarkovChain::MyOperation::node_t MarkovChain::MyOperation::preprocess_node(
	const id_t& k, neighbor_list_t& neighbors)
{
	double s = 0.0;
	for(const auto& n : neighbors)
		s += n.second;
	for(auto& n : neighbors)
		n.second /= s;
	return make_node(k, impl->fun(), neighbors);
}
MarkovChain::value_t MarkovChain::MyOperation::func(const node_t& n, const neighbor_t& neighbor){
	return n.v * neighbor.second;
}
// scheduling - priority
priority_t MarkovChain::MyOperation::priority(const node_t& n){
	double p = abs<double>(n.v - n.u);
	return static_cast<priority_t>(p * (use_degree ? n.onb.size() : 1));
}

// <source> <use-degree-priority>
AppArguments MarkovChain::MySeparator::separate(const std::vector<std::string>& args){
	AppArguments res;
	res.name = MarkovChain::name;
	res.operation_arg = {args[0], args[1]};
	res.iohandler_arg = {};
	res.progressor_arg = {};
	return res;
}
