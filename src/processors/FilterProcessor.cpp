#include "FilterProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson FilterProcessor::defaultOptions() const {
	return {
		{"min_length_mm", 0.0},
		{"max_length_mm", 1.0e9},
		{"closed_only", false},
		{"open_only", false},
		{"closed_tolerance_mm", 0.05}
	};
}

void FilterProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float minLen = options.value("min_length_mm", 0.0f);
	const float maxLen = options.value("max_length_mm", 1.0e9f);
	const bool closedOnly = options.value("closed_only", false);
	const bool openOnly = options.value("open_only", false);
	const float closedTol = options.value("closed_tolerance_mm", 0.05f);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<ofPolyline> newPaths;
	std::vector<StrokeMeta> newMeta;
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		const float len = doc.pathLengthMM(i);
		const bool closed = isPathClosed(doc.paths[i], closedTol) || doc.meta[i].closed;
		if (len < minLen || len > maxLen) continue;
		if (closedOnly && !closed) continue;
		if (openOnly && closed) continue;
		newPaths.push_back(doc.paths[i]);
		StrokeMeta m = doc.meta[i];
		m.closed = closed;
		newMeta.push_back(m);
	}
	doc.paths = std::move(newPaths);
	doc.meta = std::move(newMeta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Filtered to " + std::to_string(out->after.pathCount) + " paths";
	}
}

} // namespace plotproc
