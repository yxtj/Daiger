#pragma once

struct ProgressReport{
	double sum; // summation of the non-infinity progress value
	size_t n_inf; // # of infinity progress values
	size_t n_change; // # of changed nodes
};
