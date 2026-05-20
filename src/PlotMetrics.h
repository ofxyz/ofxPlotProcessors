#pragma once

#include "PlotProcessorTypes.h"
#include "StrokeDocument.h"

namespace plotproc {

class PlotMetricsUtil {
public:
	static PlotMetrics compute(const StrokeDocument& doc);
};

} // namespace plotproc
