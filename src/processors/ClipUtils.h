#pragma once

#include "../StrokeDocument.h"
#include <glm/glm.hpp>
#include <vector>

namespace plotproc {

std::vector<ofPolyline> cropPolylineToRect(const ofPolyline& pl, float x, float y, float w, float h);
std::vector<ofPolyline> cropPolylineToCircle(const ofPolyline& pl, const glm::vec2& center, float radiusMm);

} // namespace plotproc
