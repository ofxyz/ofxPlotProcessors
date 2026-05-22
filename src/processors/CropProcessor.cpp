#include "CropProcessor.h"
#include "ClipUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson CropProcessor::defaultOptions() const {
	return {
		{"x_mm", 0.0},
		{"y_mm", 0.0},
		{"width_mm", 100.0},
		{"height_mm", 100.0}
	};
}

void CropProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float x = options.value("x_mm", 0.0f);
	const float y = options.value("y_mm", 0.0f);
	const float w = options.value("width_mm", 100.0f);
	const float h = options.value("height_mm", 100.0f);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<ofPolyline> newPaths;
	std::vector<StrokeMeta> newMeta;

	for (size_t i = 0; i < doc.paths.size(); ++i) {
		auto parts = cropPolylineToRect(doc.paths[i], x, y, w, h);
		for (auto& p : parts) {
			newPaths.push_back(std::move(p));
			newMeta.push_back(doc.meta[i]);
		}
	}

	doc.paths = std::move(newPaths);
	doc.meta = std::move(newMeta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Cropped to rect";
	}
}

} // namespace plotproc
