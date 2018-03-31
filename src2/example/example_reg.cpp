#include "example_reg.h"
#include "cc.h"
#include "pr.h"
#include "mc.h"
#include "sssp.h"
#include "sswp.h"

using namespace std;

void registerExamples(){
	ConnectedComponent().reg();
	PageRank().reg();
	MarkovChain().reg();
	ShortestPath().reg();
	WidestPath().reg();
}