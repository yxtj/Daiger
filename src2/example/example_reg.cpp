#include "example_reg.h"
#include "cc.h"
#include "mc.h"
#include "pr.h"
#include "sssp.h"
#include "sswp.h"
#include "katz.h"
//#include "adsorption.h"
//#include "jacobi.h"

using namespace std;

void registerExamples() {
    ConnectedComponent().reg();
    PageRank().reg();
    MarkovChain().reg();
    ShortestPath().reg();
    WidestPath().reg();
    Katz().reg();
	//Adsorption().reg();
	//Jacobi().reg();
}