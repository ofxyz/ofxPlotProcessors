#include "ReloopProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <random>

namespace plotproc {

ofJson ReloopProcessor::defaultOptions() const {
	return {
		{"tolerance_mm", 0.05},
		{"seed", -1}
	};
}

void ReloopProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float tol = options.value("tolerance_mm", 0.05f);
	const int seed = options.value("seed", -1);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	std::mt19937 rng(seed >= 0 ? (uint32_t)seed : (uint32_t)ofGetElapsedTimef() * 1000.f);
	std::uniform_int_distribution<int> dist;

	doc.syncMetaSize();
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		auto& pl = doc.paths[i];
		if (pl.size() < 3) continue;
		if (!isPathClosed(pl, tol) && !doc.meta[i].closed) continue;

		const int loc = dist(rng, std::uniform_int_distribution<int>::param_type(0, (int)pl.size() - 2));
		glm::vec3 mid = 0.5f * (pl[0] + pl[pl.size() - 1]);

		ofPolyline relooped;
		relooped.addVertex(mid);
		for (int k = 0; k < (int)pl.size() - 1; ++k) {
			const size_t idx = (size_t)((loc + k) % (pl.size() - 1));
			relooped.addVertex(pl[idx]);
		}
		relooped.addVertex(mid);
		relooped.setClosed(true);
		pl = relooped;
		doc.meta[i].closed = true;
	}

	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Relooped closed paths (vpype reloop)";
	}
}

} // namespace plotproc
