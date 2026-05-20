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
		const ofPolyline& line = doc.paths[i];
		if (line.size() < 2) continue;

		ofPolyline combined = line;
		for (int pass = 0; pass < count - 1; ++pass) {
			if (pass % 2 == 0) {
				ofPolyline rev;
				const auto& verts = line.getVertices();
				for (int vi = (int)verts.size() - 2; vi >= 0; --vi) {
					rev.addVertex(verts[vi]);
				}
				appendPolylineJoin(combined, rev, false);
			} else {
				ofPolyline tail;
				for (size_t vi = 1; vi < line.size(); ++vi) {
					tail.addVertex(line[vi]);
				}
				appendPolylineJoin(combined, tail, false);
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
