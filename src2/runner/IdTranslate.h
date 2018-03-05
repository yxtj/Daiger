#pragma once
#include <utility>

// return <whether-a-worker, worker/master-id>
std::pair<bool, int> nidtrans(const int id);
