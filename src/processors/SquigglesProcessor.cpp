#include "SquigglesProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson SquigglesProcessor::defaultOptions() const {
	return {
		{"amplitude_mm", 0.5},
		{"period_mm", 3.0},
		{"quantization_mm", 0.05},
		{"seed", 0}
	};
}

void SquigglesProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float amplitude = options.value("amplitude_mm", 0.5f);
	const float period = std::max(0.001f, options.value("period_mm", 3.0f));
	const float quantization = std::max(0.001f, options.value("quantization_mm", 0.05f));
	const int seed = options.value("seed", 0);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	ofSeedRandom(seed);

	for (size_t pi = 0; pi < doc.paths.size(); ++pi) {
		ofPolyline resampled = resamplePolyline(pathToPolyline(doc.paths[pi]), quantization);
		ofPolyline wobbled;
		wobbled.setClosed(resampled.isClosed());
		for (const auto& v : resampled.getVertices()) {
			const float nx = ofNoise(v.x / period, v.y / period, seed * 0.17f) * 2.f - 1.f;
			const float ny = ofNoise(v.x / period + 31.7f, v.y / period + 91.2f, seed * 0.23f) * 2.f - 1.f;
			wobbled.addVertex(v.x + nx * amplitude, v.y + ny * amplitude, v.z);
		}
		doc.paths[pi] = polylineToPath(wobbled);
	}

	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Applied squiggles";
	}
}

} // namespace plotproc
