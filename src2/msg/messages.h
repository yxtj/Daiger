#pragma once
#include "common/def.h"
#include <string>
#include <utility>
#include <tuple>
#include <vector>

enum ProcedureType : int {
	None = 0,
	LoadGraph = 1,
	LoadValue = 2,
	LoadDelta = 3,
	Update = 4,
	Output = 5
};

using CommonMsg_t = std::tuple<id_t, id_t, value_t>; // src, dst, value

using GINCache_t = CommonMsg_t;
using MsgGINCache_t = std::vector<GINCache_t>;

using VUpdate_t = CommonMsg_t;
using MsgVUpdate_t = std::vector<VUpdate_t>;

using VRequest_t = std::pair<id_t, id_t>; // asker, receiver
using MsgVRequest_t = std::vector<VRequest_t>;

using VReply_t = CommonMsg_t; // asker, receiver, value
using MsgVReply_t std::vector<VReply_t>;
