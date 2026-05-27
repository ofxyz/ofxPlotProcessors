#include "LineMergeProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson LineMergeProcessor::defaultOptions() const {
	return {
		{"tolerance_mm", 0.05},
		{"allow_reverse", true}
	};
}

void LineMergeProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float tol = options.value("tolerance_mm", 0.05f);
	const bool allowFlip = options.value("allow_reverse", true);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	StrokeDocument rebuilt;
	rebuilt.paths.reserve(doc.paths.size());
	rebuilt.meta.reserve(doc.meta.size());

	for (int layerId : layerIdsInOrder(doc)) {
		std::vector<ofPath> mergeable;
		std::vector<StrokeMeta> mergeMeta;
		std::vector<ofPath> lockedPaths;
		std::vector<StrokeMeta> lockedMeta;

		for (size_t i = 0; i < doc.paths.size(); ++i) {
			if (doc.meta[i].layerId != layerId) continue;
			if (doc.meta[i].locked || pathIsEmpty(doc.paths[i])) {
				lockedPaths.push_back(doc.paths[i]);
				lockedMeta.push_back(doc.meta[i]);
				continue;
			}
			mergeable.push_back(doc.paths[i]);
			mergeMeta.push_back(doc.meta[i]);
		}

		if (mergeable.size() >= 2) {
			mergePathsVpype(mergeable, tol, allowFlip);
		}

		for (size_t i = 0; i < lockedPaths.size(); ++i) {
			rebuilt.paths.push_back(std::move(lockedPaths[i]));
			rebuilt.meta.push_back(lockedMeta[i]);
		}
		const size_t n = std::min(mergeable.size(), mergeMeta.size());
		for (size_t i = 0; i < n; ++i) {
			rebuilt.paths.push_back(std::move(mergeable[i]));
			rebuilt.meta.push_back(mergeMeta[i]);
		}
	}

	doc.paths = std::move(rebuilt.paths);
	doc.meta = std::move(rebuilt.meta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Merged (vpype linemerge) to " + std::to_string(out->after.pathCount) + " paths";
	}
}

} // namespace plotproc
