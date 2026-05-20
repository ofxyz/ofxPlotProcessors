#include "StrokeDocument.h"
#include <limits>

namespace plotproc {

void StrokeDocument::syncMetaSize() {
	if (meta.size() < paths.size()) {
		meta.resize(paths.size());
	} else if (meta.size() > paths.size()) {
		meta.resize(paths.size());
	}
	for (size_t i = 0; i < paths.size(); ++i) {
		if (paths[i].size() >= 2) {
			meta[i].closed = paths[i].isClosed();
		}
	}
}

void StrokeDocument::rebuildBounds() {
	if (paths.empty()) {
		bounds = {};
		return;
	}
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();
	for (const auto& pl : paths) {
		for (const auto& v : pl.getVertices()) {
			minX = std::min(minX, v.x);
			minY = std::min(minY, v.y);
			maxX = std::max(maxX, v.x);
			maxY = std::max(maxY, v.y);
		}
	}
	bounds.set(minX, minY, maxX - minX, maxY - minY);
}

size_t StrokeDocument::vertexCount() const {
	size_t n = 0;
	for (const auto& pl : paths) {
		n += pl.size();
	}
	return n;
}

float StrokeDocument::pathLengthMM(size_t index) const {
	if (index >= paths.size()) return 0.f;
	return paths[index].getPerimeter();
}

glm::vec2 StrokeDocument::pathStart(size_t index) const {
	if (index >= paths.size() || paths[index].size() == 0) return {};
	const auto& v = paths[index].getVertices().front();
	return {v.x, v.y};
}

glm::vec2 StrokeDocument::pathEnd(size_t index) const {
	if (index >= paths.size() || paths[index].size() == 0) return {};
	const auto& v = paths[index].getVertices().back();
	return {v.x, v.y};
}

} // namespace plotproc
