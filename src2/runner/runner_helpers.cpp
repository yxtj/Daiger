#include "runner_helpers.h"

using namespace std;

// return <whether-a-worker, worker/master-id>
std::pair<bool, int> nidtrans(const int id){
	if(id == 0){
		return make_pair(false, 0);
	}else{
		return make_pair(true, id-1);
	}		
}
