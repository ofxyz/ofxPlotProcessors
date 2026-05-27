#include "ScaleToProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <algorithm>

namespace plotproc {

ofJson ScaleToProcessor::defaultOptions() const {
	return {
		{"width_mm", 100.0},
		{"height_mm", 100.0},
		{"fit_dimensions", false}
	};
}

void ScaleToProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float targetW = std::max(1e-6f, options.value("width_mm", 100.0f));
	const float targetH = std::max(1e-6f, options.value("height_mm", 100.0f));
	const bool fitDimensions = options.value("fit_dimensions", false);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<size_t> affected;
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		if (!pathIsEmpty(doc.paths[i])) affected.push_back(i);
	}

	const ofRectangle r = boundsForPathIndices(doc, affected);
	if (r.width < 1e-6f && r.height < 1e-6f) {
		if (out) {
			out->before = before;
			out->after = before;
			out->log = "Scale to: empty bounds";
		}
		return;
	}

	float sx = targetW / std::max(1e-6f, r.width);
	float sy = targetH / std::max(1e-6f, r.height);
	if (!fitDimensions) {
		const float s = std::min(sx, sy);
		sx = sy = s;
	}

	const glm::vec2 origin = resolveTransformOrigin(doc, options);
	for (size_t i : affected) {
		scalePathAtOrigin(doc.paths[i], origin, sx, sy);
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = fitDimensions ? "Scaled to fit dimensions" : "Scaled uniformly to target size";
	}
}

} // namespace plotproc
