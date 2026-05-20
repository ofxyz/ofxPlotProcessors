#include "PlotMetrics.h"
#include <glm/glm.hpp>

namespace plotproc {

PlotMetrics PlotMetricsUtil::compute(const StrokeDocument& doc) {
	PlotMetrics m;
	m.pathCount = (int)doc.paths.size();
	m.vertexCount = (int)doc.vertexCount();
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		m.drawLengthMM += doc.pathLengthMM(i);
	}
	for (size_t i = 1; i < doc.paths.size(); ++i) {
		const glm::vec2 a = doc.pathEnd(i - 1);
		const glm::vec2 b = doc.pathStart(i);
		m.travelLengthMM += glm::length(b - a);
	}
	return m;
}

} // namespace plotproc
