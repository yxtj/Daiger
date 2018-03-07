#pragma once
#include <string>

enum ProcedureType : int {
	None = 0,
	LoadGraph = 1,
	LoadValue = 2,
	LoadDelta = 3,
	Update = 4,
	Output = 5
};
