#include "RotateProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson RotateProcessor::defaultOptions() const {
	return {{"angle_deg", 0.0}};
}

void RotateProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float angle = options.value("angle_deg", 0.0f);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	const glm::vec2 origin = resolveTransformOrigin(doc, options);
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		rotateVerticesClockwise(doc.paths[i], origin, angle);
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Rotated " + ofToString(angle, 2) + " deg";
	}
}

} // namespace plotproc
