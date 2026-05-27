#include "SnapProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <cmath>

namespace plotproc {

ofJson SnapProcessor::defaultOptions() const {
	return {{"pitch_mm", 1.0}};
}

void SnapProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float pitch = std::max(1e-4f, options.value("pitch_mm", 1.0f));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	for (size_t i = 0; i < doc.paths.size(); ++i) {
		ofPolyline pl = pathToPolyline(doc.paths[i]);
		if (pl.size() < 2) continue;

		ofPolyline snapped;
		snapped.setClosed(pl.isClosed());
		for (const auto& v : pl.getVertices()) {
			const float sx = std::round(v.x / pitch) * pitch;
			const float sy = std::round(v.y / pitch) * pitch;
			if (snapped.size() > 0) {
				const auto& prev = snapped.getVertices().back();
				if (std::abs(prev.x - sx) < 1e-6f && std::abs(prev.y - sy) < 1e-6f) continue;
			}
			snapped.addVertex(sx, sy, v.z);
		}
		if (snapped.size() >= 2) {
			doc.paths[i] = polylineToPath(snapped);
		}
	}

	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Snapped to " + ofToString(pitch, 3) + " mm grid";
	}
}

} // namespace plotproc
