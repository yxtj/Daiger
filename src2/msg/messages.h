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
	BuildINCache = 4,
	Update = 5,
	DumpResult = 6
};

template <typename V>
struct MessageDef {
	using value_t = V;
	using CommonMsg_t = std::tuple<id_t, id_t, value_t>; // src, dst, value

	using GINCache_t = CommonMsg_t;
	using MsgGINCache_t = std::vector<GINCache_t>;

	using VUpdate_t = CommonMsg_t;
	using MsgVUpdate_t = std::vector<VUpdate_t>;

	using VRequest_t = std::pair<id_t, id_t>; // asker, parent
	//using MsgVRequest_t = std::vector<VRequest_t>;

	using VReply_t = CommonMsg_t; // asker, parent, value
	//using MsgVReply_t = std::vector<VReply_t>;
};
