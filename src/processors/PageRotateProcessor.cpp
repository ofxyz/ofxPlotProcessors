#include "PageRotateProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson PageRotateProcessor::defaultOptions() const {
	return {{"clockwise", false}};
}

void PageRotateProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const bool clockwise = options.value("clockwise", false);
	const float angle = clockwise ? -90.f : 90.f;

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	const glm::vec2 origin = resolveTransformOrigin(doc, options);
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		rotatePathClockwise(doc.paths[i], origin, angle);
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = clockwise ? "Rotated 90° clockwise" : "Rotated 90° counter-clockwise";
	}
}

} // namespace plotproc
