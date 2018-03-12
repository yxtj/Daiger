#pragma once
#include <cstdint>
#include <utility>
//#include <vector>

typedef uint32_t id_t;
//typedef float value_t;
typedef float priority_t;

//typedef std::pair<id_t, value_t> neighbor_t;
//typedef std::vector<neighbor_t> neighbor_list_t;

enum class ChangeEdgeType: char{
	ADD='A',
	REMOVE='R',
	INCREASE='I',
	DECREASE='D'
};

// N can only be "id_t" or "std::pair<id_t, W>"
template <typename N = std::pair<id_t, float> >
struct ChangeEdge {
	ChangeEdgeType type;
	id_t src;
	N dst;
};
