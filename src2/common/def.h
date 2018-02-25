#pragma once
#include <cstdint>
#include <string>
//#include <vector>

typedef uint32_t key_t;
//typedef float value_t;
typedef float priority_t;

key_t stok(const std::string& str){
	return static_cast<key_t>(std::stoul(str));
}
priority_t stop(const std::string& str){
	return std::stof(str);
}

//typedef std::pair<key_t, value_t> neighbor_t;
//typedef std::vector<neighbor_t> neighbor_list_t;

enum class ChangeEdgeType: char{
	ADD='A',
	REMOVE='R',
	INCREASE='I',
	DECREASE='D'
};

struct change_t {
	ChangeEdgeType type;
	key_t src, dst;
	float weight;
};
