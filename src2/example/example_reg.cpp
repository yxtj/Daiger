#include "example_reg.h"
#include "cc.h"
#include "pr.h"
#include "sssp.h"

using namespace std;

void registerExamples(){
	ConnectedComponent().reg();
	PageRank().reg();
	ShortestPath().reg();
}