#include "SkewProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson SkewProcessor::defaultOptions() const {
	return {{"skew_x_deg", 0.0}, {"skew_y_deg", 0.0}};
}

void SkewProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float skewX = options.value("skew_x_deg", 0.0f);
	const float skewY = options.value("skew_y_deg", options.value("skew_deg", 0.0f));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	const glm::vec2 origin = resolveTransformOrigin(doc, options);
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		skewPathAtOrigin(doc.paths[i], origin, skewX, skewY);
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Skewed geometries";
	}
}

} // namespace plotproc
