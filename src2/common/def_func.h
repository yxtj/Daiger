#pragma once
#include "def.h"
#include <string>

id_t stoid(const std::string& str);

priority_t stop(const std::string& str);

// generate an id (NORMALLY unused, usually negative) for a dummy node, given its dummy-sequence-id
id_t gen_dummy_id(const size_t id);
