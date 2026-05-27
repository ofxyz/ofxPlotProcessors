#include "CircleCropProcessor.h"
#include "ClipUtils.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson CircleCropProcessor::defaultOptions() const {
	return {{"center_x_mm", 0.0}, {"center_y_mm", 0.0}, {"radius_mm", 50.0}};
}

void CircleCropProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const glm::vec2 center = {
		options.value("center_x_mm", 0.0f),
		options.value("center_y_mm", 0.0f)
	};
	const float radius = std::max(1e-4f, options.value("radius_mm", 50.0f));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<ofPath> newPaths;
	std::vector<StrokeMeta> newMeta;

	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) {
			newPaths.push_back(doc.paths[i]);
			newMeta.push_back(doc.meta[i]);
			continue;
		}
		auto parts = cropPolylineToCircle(pathToPolyline(doc.paths[i]), center, radius);
		for (auto& p : parts) {
			newPaths.push_back(polylineToPath(p));
			newMeta.push_back(i < doc.meta.size() ? doc.meta[i] : StrokeMeta{});
		}
	}

	doc.paths = std::move(newPaths);
	doc.meta = std::move(newMeta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Circle cropped";
	}
}

} // namespace plotproc
