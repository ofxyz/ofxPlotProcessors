#include "LineSortProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <limits>
#include <map>
#include <vector>

namespace plotproc {

ofJson LineSortProcessor::defaultOptions() const {
	return {
		{"allow_reverse", true},
		{"two_opt", false},
		{"passes", 1}
	};
}

namespace {

void twoOptPass(std::vector<ofPolyline>& lines, int passes) {
	if (lines.size() < 3) return;

	auto endpointCost = [](const glm::vec2& from, const glm::vec2& to) {
		return glm::length(to - from);
	};

	for (int pass = 0; pass < passes; ++pass) {
		bool improved = false;
		for (size_t i = 1; i + 1 < lines.size(); ++i) {
			const glm::vec2 endA = {lines[i - 1].getVertices().back().x,
			                        lines[i - 1].getVertices().back().y};
			const glm::vec2 startB = {lines[i].getVertices().front().x,
			                          lines[i].getVertices().front().y};
			const glm::vec2 endB = {lines[i].getVertices().back().x,
			                        lines[i].getVertices().back().y};
			const glm::vec2 startC = {lines[i + 1].getVertices().front().x,
			                          lines[i + 1].getVertices().front().y};
			const float beforeCost = endpointCost(endA, startB) + endpointCost(endB, startC);
			const float afterCost = endpointCost(endA, startC) + endpointCost(endB, startB);
			if (afterCost + 1e-4f < beforeCost) {
				std::swap(lines[i], lines[i + 1]);
				improved = true;
			}
		}
		if (!improved) break;
	}
}

} // namespace

void LineSortProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const bool allowFlip = options.value("allow_reverse", true);
	const bool twoOpt = options.value("two_opt", false);
	const int passes = std::max(1, options.value("passes", 1));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	StrokeDocument rebuilt;
	rebuilt.paths.reserve(doc.paths.size());
	rebuilt.meta.reserve(doc.meta.size());

	for (int layerId : layerIdsInOrder(doc)) {
		std::vector<ofPolyline> sortable;
		std::vector<StrokeMeta> sortMeta;
		std::vector<ofPolyline> lockedPaths;
		std::vector<StrokeMeta> lockedMeta;

		for (size_t i = 0; i < doc.paths.size(); ++i) {
			if (doc.meta[i].layerId != layerId) continue;
			if (doc.meta[i].locked) {
				lockedPaths.push_back(doc.paths[i]);
				lockedMeta.push_back(doc.meta[i]);
				continue;
			}
			if (doc.paths[i].size() >= 2) {
				sortable.push_back(doc.paths[i]);
				sortMeta.push_back(doc.meta[i]);
			}
		}

		if (sortable.size() >= 2) {
			sortPolylinesGreedyVpype(sortable, allowFlip);
			if (twoOpt) twoOptPass(sortable, passes);
		}

		for (size_t i = 0; i < lockedPaths.size(); ++i) {
			rebuilt.paths.push_back(std::move(lockedPaths[i]));
			rebuilt.meta.push_back(lockedMeta[i]);
		}
		const size_t n = std::min(sortable.size(), sortMeta.size());
		for (size_t i = 0; i < n; ++i) {
			rebuilt.paths.push_back(std::move(sortable[i]));
			rebuilt.meta.push_back(sortMeta[i]);
		}
	}

	doc.paths = std::move(rebuilt.paths);
	doc.meta = std::move(rebuilt.meta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Sorted (vpype linesort) " + std::to_string(out->after.pathCount) + " strokes";
	}
}

} // namespace plotproc
