#pragma once
#include "common/def.h"
#include <string>
#include <utility>
#include <tuple>
#include <vector>

enum ProcedureType : int {
	None = -1,
	ShareWorkers = 0,
	LoadGraph = 1,
	LoadValue = 2,
	BuildINList = 3,
	BuildINCache = 4,
	RebuildStructure = 5,
	LoadDelta = 6,
	GenInitMsg = 7,
	Update = 8,
	DumpResult = 9
};

template <typename V, typename N>
struct MessageDef {
	using value_t = V;
	using neighbor_t = N;
	using neighbor_list_t = std::vector<N>;
	using CommonMsg_t = std::tuple<id_t, id_t, value_t>; // src, dst, value

	using GINList_t = std::pair<id_t, neighbor_list_t>;
	using MsgGINList_t = std::vector<GINList_t>;

	using GINCache_t = CommonMsg_t;
	using MsgGINCache_t = std::vector<GINCache_t>;

	using VUpdate_t = CommonMsg_t;
	using MsgVUpdate_t = std::vector<VUpdate_t>;

	using VRequest_t = std::pair<id_t, id_t>; // asker, parent
	//using MsgVRequest_t = std::vector<VRequest_t>;

	using VReply_t = CommonMsg_t; // asker, parent, value
	//using MsgVReply_t = std::vector<VReply_t>;
};
