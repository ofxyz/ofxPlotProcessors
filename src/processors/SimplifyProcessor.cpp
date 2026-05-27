#include "SimplifyProcessor.h"
#include "../PlotMetrics.h"

namespace plotproc {

ofJson SimplifyProcessor::defaultOptions() const {
	return {{"tolerance_mm", 0.1}};
}

void SimplifyProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float tol = options.value("tolerance_mm", 0.1f);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	for (auto& p : doc.paths) {
		ofPolyline pl = pathToPolyline(p);
		if (pl.size() >= 3) {
			pl.simplify(tol);
			p = polylineToPath(pl);
		}
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Simplified paths";
	}
}

} // namespace plotproc
