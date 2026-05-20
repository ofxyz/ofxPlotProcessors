#pragma once

#include "ofMain.h"
#include <vector>

namespace plotproc {

struct StrokeMeta {
	bool closed = false;
	bool allowReverse = true;
	bool locked = false;
	int layerId = 0;
	ofColor color;
	/// Source layer entity (entt) when built by ofxPlotter bridge; 0 = unknown.
	uint32_t layerEntityRaw = 0;
	int sourcePathIndex = -1;
};

struct StrokeDocument {
	std::vector<ofPolyline> paths;
	std::vector<StrokeMeta> meta;

	ofRectangle bounds;

	void syncMetaSize();
	void rebuildBounds();
	size_t vertexCount() const;
	float pathLengthMM(size_t index) const;
	glm::vec2 pathStart(size_t index) const;
	glm::vec2 pathEnd(size_t index) const;
};

} // namespace plotproc
