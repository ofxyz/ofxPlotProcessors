#include "SplitDistProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <algorithm>

namespace plotproc {

ofJson SplitDistProcessor::defaultOptions() const {
	return {{"max_draw_mm", 1000.0}};
}

void SplitDistProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float maxDraw = std::max(1e-3f, options.value("max_draw_mm", 1000.0f));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	int maxLayerId = 0;
	for (const auto& m : doc.meta) {
		maxLayerId = std::max(maxLayerId, m.layerId);
	}

	for (int layerId : layerIdsInOrder(doc)) {
		float cumulative = 0.f;
		int targetLayer = layerId;
		for (size_t i = 0; i < doc.paths.size(); ++i) {
			if (doc.meta[i].layerId != layerId) continue;
			if (!pathMatchesLayerFilter(doc.meta[i], options)) continue;
			const float len = doc.pathLengthMM(i);
			if (cumulative > 0.f && cumulative + len > maxDraw) {
				targetLayer = ++maxLayerId;
				cumulative = 0.f;
			}
			doc.meta[i].layerId = targetLayer;
			cumulative += len;
		}
	}

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Split layers by draw distance";
	}
}

} // namespace plotproc
