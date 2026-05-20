#pragma once

#include "ofMain.h"
#include <string>

namespace plotproc {

enum class ProcessorScope {
	PerDocument,
	PerLayer,
	PerStroke
};

struct PlotMetrics {
	int   pathCount    = 0;
	int   vertexCount  = 0;
	float drawLengthMM = 0.f;
	float travelLengthMM = 0.f;
};

struct ProcessorResult {
	PlotMetrics before;
	PlotMetrics after;
	std::string log;
};

} // namespace plotproc
