#pragma once

#include "../StrokeDocument.h"
#include <glm/glm.hpp>
#include <vector>

namespace plotproc {

inline float dist2d(const glm::vec2& a, const glm::vec2& b) {
	return glm::length(b - a);
}

void reversePolyline(ofPolyline& pl);
ofPolyline resamplePolyline(const ofPolyline& src, float maxSegmentMm);

void appendPolylineJoin(ofPolyline& dst, const ofPolyline& src, bool reverseSrc);
void prependPolylineJoin(ofPolyline& dst, const ofPolyline& src, bool reverseSrc);

/// vpype ``LineCollection.merge`` on a single layer's polylines.
void mergePolylinesVpype(std::vector<ofPolyline>& lines, float toleranceMm, bool allowFlip);

/// vpype ``linesort`` greedy pass. Reorders @p lines in place when travel improves.
/// Returns false if original order was kept.
bool sortPolylinesGreedyVpype(std::vector<ofPolyline>& lines, bool allowFlip);

/// Layer ids in first-seen order.
std::vector<int> layerIdsInOrder(const StrokeDocument& doc);

/// Path indices belonging to one layer.
std::vector<size_t> pathIndicesForLayer(const StrokeDocument& doc, int layerId);

bool isPathClosed(const ofPolyline& pl, float toleranceMm);

} // namespace plotproc
