#include "LineShuffleProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <algorithm>
#include <random>

namespace plotproc {

ofJson LineShuffleProcessor::defaultOptions() const {
	return {{"seed", -1}};
}

void LineShuffleProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const int seed = options.value("seed", -1);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::mt19937 rng(seed >= 0 ? (uint32_t)seed : (uint32_t)ofGetElapsedTimef() * 1000.f);

	StrokeDocument shuffled;
	for (int layerId : layerIdsInOrder(doc)) {
		std::vector<size_t> indices;
		for (size_t i = 0; i < doc.paths.size(); ++i) {
			if (doc.meta[i].layerId == layerId) indices.push_back(i);
		}
		if (layerMatchesLayerFilter(layerId, options)) {
			std::shuffle(indices.begin(), indices.end(), rng);
		}
		for (size_t i : indices) {
			shuffled.paths.push_back(doc.paths[i]);
			shuffled.meta.push_back(doc.meta[i]);
		}
	}

	doc.paths = std::move(shuffled.paths);
	doc.meta = std::move(shuffled.meta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Shuffled stroke order per layer";
	}
}

} // namespace plotproc
