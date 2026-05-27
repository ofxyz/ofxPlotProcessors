#include "SplitAllProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson SplitAllProcessor::defaultOptions() const {
	return ofJson::object();
}

void SplitAllProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	(void)options;
	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<ofPath> newPaths;
	std::vector<StrokeMeta> newMeta;

	for (size_t i = 0; i < doc.paths.size(); ++i) {
		const ofPolyline pl = pathToPolyline(doc.paths[i]);
		if (pl.size() < 2) continue;
		const auto& verts = pl.getVertices();
		for (size_t vi = 0; vi + 1 < verts.size(); ++vi) {
			if (verts[vi] == verts[vi + 1]) continue;
			ofPath seg;
			seg.moveTo(verts[vi]);
			seg.lineTo(verts[vi + 1]);
			newPaths.push_back(std::move(seg));
			newMeta.push_back(doc.meta[i]);
		}
	}

	doc.paths = std::move(newPaths);
	doc.meta = std::move(newMeta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Split to " + std::to_string(out->after.pathCount) + " segments";
	}
}

} // namespace plotproc
