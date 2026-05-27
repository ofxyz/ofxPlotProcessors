#include "ScaleProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson ScaleProcessor::defaultOptions() const {
	return {{"scale_x", 1.0}, {"scale_y", 1.0}};
}

void ScaleProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float sx = options.value("scale_x", options.value("scale", 1.0f));
	const float sy = options.value("scale_y", options.value("scale", sx));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	const glm::vec2 origin = resolveTransformOrigin(doc, options);
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		scalePathAtOrigin(doc.paths[i], origin, sx, sy);
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Scaled " + ofToString(sx, 4) + " x " + ofToString(sy, 4);
	}
}

} // namespace plotproc
