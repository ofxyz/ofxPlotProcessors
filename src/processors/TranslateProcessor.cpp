#include "TranslateProcessor.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson TranslateProcessor::defaultOptions() const {
	return {{"offset_x_mm", 0.0}, {"offset_y_mm", 0.0}};
}

void TranslateProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float ox = options.value("offset_x_mm", 0.0f);
	const float oy = options.value("offset_y_mm", 0.0f);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	for (auto& p : doc.paths) {
		transformPathInPlace(p, [ox, oy](const glm::vec2& v) {
			return glm::vec2{v.x + ox, v.y + oy};
		});
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Translated";
	}
}

} // namespace plotproc
