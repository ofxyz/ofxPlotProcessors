#include "TrimProcessor.h"
#include "ClipUtils.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson TrimProcessor::defaultOptions() const {
	return {{"margin_x_mm", 1.0}, {"margin_y_mm", 1.0}};
}

void TrimProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float mx = std::max(0.f, options.value("margin_x_mm", 1.0f));
	const float my = std::max(0.f, options.value("margin_y_mm", 1.0f));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<size_t> affected;
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		if (doc.paths[i].size() > 0) affected.push_back(i);
	}

	const ofRectangle r = boundsForPathIndices(doc, affected);
	if (r.width < 2.f * mx || r.height < 2.f * my) {
		if (out) {
			out->before = before;
			out->after = before;
			out->log = "Trim: margins too large for bounds";
		}
		return;
	}

	const float x = r.x + mx;
	const float y = r.y + my;
	const float w = r.width - 2.f * mx;
	const float h = r.height - 2.f * my;

	std::vector<ofPolyline> newPaths;
	std::vector<StrokeMeta> newMeta;
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) {
			newPaths.push_back(doc.paths[i]);
			newMeta.push_back(doc.meta[i]);
			continue;
		}
		auto parts = cropPolylineToRect(doc.paths[i], x, y, w, h);
		for (auto& p : parts) {
			newPaths.push_back(std::move(p));
			newMeta.push_back(i < doc.meta.size() ? doc.meta[i] : StrokeMeta{});
		}
	}

	doc.paths = std::move(newPaths);
	doc.meta = std::move(newMeta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Trimmed by margin";
	}
}

} // namespace plotproc
