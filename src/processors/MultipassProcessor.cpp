#include "MultipassProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson MultipassProcessor::defaultOptions() const {
	return {{"count", 2}};
}

void MultipassProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const int count = options.value("count", 2);
	if (count < 2) return;

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	for (size_t i = 0; i < doc.paths.size(); ++i) {
		const ofPolyline line = pathToPolyline(doc.paths[i]);
		if (line.size() < 2) continue;

		ofPath combined = polylineToPath(line);
		for (int pass = 0; pass < count - 1; ++pass) {
			if (pass % 2 == 0) {
				// Reversed pass
				ofPolyline rev;
				const auto& verts = line.getVertices();
				for (int vi = (int)verts.size() - 2; vi >= 0; --vi) {
					rev.addVertex(verts[vi]);
				}
				for (size_t vi = 1; vi < rev.size(); ++vi) {
					combined.lineTo(rev[vi]);
				}
			} else {
				// Forward pass (skip first vertex)
				for (size_t vi = 1; vi < line.size(); ++vi) {
					combined.lineTo(line[vi]);
				}
			}
		}
		doc.paths[i] = combined;
	}

	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Multipass x" + std::to_string(count);
	}
}

} // namespace plotproc
