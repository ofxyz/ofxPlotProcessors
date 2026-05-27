#pragma once

#include "../StrokeDocument.h"
#include <functional>
#include <glm/glm.hpp>
#include <vector>

namespace plotproc {

inline float dist2d(const glm::vec2& a, const glm::vec2& b) {
	return glm::length(b - a);
}

// ---- ofPath / ofPolyline conversion helpers --------------------------------

/// First drawable point of a path (first moveTo or lineTo command).
glm::vec2 pathStartPt(const ofPath& p);

/// Last drawable point of a path (last non-close command).
glm::vec2 pathEndPt(const ofPath& p);

/// True when the path has no drawable commands.
bool pathIsEmpty(const ofPath& p);

/// Tessellate path to its first outline polyline.
ofPolyline pathToPolyline(const ofPath& p);

/// Build an ofPath containing only lineTo commands from a polyline.
ofPath polylineToPath(const ofPolyline& pl);

// ---- Affine transform helpers (preserve bezier control points) ------------

/// Apply a 2D transform to every control point in the path.
/// Arc commands are tessellated before transformation.
void transformPathInPlace(ofPath& p,
                          const std::function<glm::vec2(const glm::vec2&)>& fn);

/// Reverse the drawing direction of a path (tessellates, then rebuilds as lineTo).
void reversePath(ofPath& p);

/// Append src (optionally reversed) onto the end of dst.
/// Forward appends preserve src bezier commands; reversed appends tessellate.
void appendPathJoin(ofPath& dst, const ofPath& src, bool reverseSrc);

/// Prepend src (optionally reversed) before dst.
/// src is tessellated when reversed; dst bezier commands are always preserved.
void prependPathJoin(ofPath& dst, const ofPath& src, bool reverseSrc);

// ---- Topology helpers ------------------------------------------------------

void mergePathsVpype(std::vector<ofPath>& paths, float toleranceMm, bool allowFlip);
bool sortPathsGreedyVpype(std::vector<ofPath>& paths, bool allowFlip);

/// Layer ids in first-seen order.
std::vector<int> layerIdsInOrder(const StrokeDocument& doc);

/// Path indices belonging to one layer.
std::vector<size_t> pathIndicesForLayer(const StrokeDocument& doc, int layerId);

bool isPathClosed(const ofPath& p, float toleranceMm);

/// Bounds over path indices (empty indices → document bounds).
ofRectangle boundsForPathIndices(const StrokeDocument& doc, const std::vector<size_t>& indices);

/// If @p options contains non-empty ``layer_ids`` array, only those layers are affected.
bool pathMatchesLayerFilter(const StrokeMeta& meta, const ofJson& options);
bool layerMatchesLayerFilter(int layerId, const ofJson& options);

/// vpype-style origin: bbox centre of affected paths unless ``origin_x_mm`` / ``origin_y_mm`` set.
glm::vec2 resolveTransformOrigin(const StrokeDocument& doc, const ofJson& options);

void scalePathAtOrigin(ofPath& p, const glm::vec2& origin, float sx, float sy);
void rotatePathClockwise(ofPath& p, const glm::vec2& origin, float angleDeg);
void skewPathAtOrigin(ofPath& p, const glm::vec2& origin, float skewXDeg, float skewYDeg);

/// Resample a polyline so no segment exceeds maxSegmentMm.
ofPolyline resamplePolyline(const ofPolyline& src, float maxSegmentMm);

/// vpype paper presets (mm). Returns false if preset unknown.
bool pagePresetSizeMm(const std::string& preset, float& widthMm, float& heightMm);

} // namespace plotproc
