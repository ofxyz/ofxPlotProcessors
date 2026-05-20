#include "ReverseProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <algorithm>

namespace plotproc {

ofJson ReverseProcessor::defaultOptions() const {
	return {
		{"reverse_order", true},
		{"flip_lines", false}
	};
}

void ReverseProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const bool reverseOrder = options.value("reverse_order", true);
	const bool flipLines = options.value("flip_lines", false);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	if (reverseOrder) {
		std::reverse(doc.paths.begin(), doc.paths.end());
		std::reverse(doc.meta.begin(), doc.meta.end());
	}
	if (flipLines) {
		for (auto& pl : doc.paths) {
			reversePolyline(pl);
		}
	}

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Reversed strokes";
	}
}

} // namespace plotproc
