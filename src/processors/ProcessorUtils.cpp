#include "ProcessorUtils.h"
#include "../LineIndex.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <string>

namespace plotproc {

void reversePolyline(ofPolyline& pl) {
	if (pl.size() < 2) return;
	std::vector<glm::vec3> verts = pl.getVertices();
	std::reverse(verts.begin(), verts.end());
	pl.clear();
	for (const auto& v : verts) {
		pl.addVertex(v);
	}
	pl.setClosed(pl.isClosed());
}

ofPolyline resamplePolyline(const ofPolyline& src, float maxSegmentMm) {
	ofPolyline out;
	if (src.size() < 2 || maxSegmentMm <= 0.f) {
		return src;
	}
	const auto& verts = src.getVertices();
	out.addVertex(verts.front());
	for (size_t i = 1; i < verts.size(); ++i) {
		glm::vec3 a = verts[i - 1];
		glm::vec3 b = verts[i];
		const float segLen = glm::length(glm::vec2(b.x - a.x, b.y - a.y));
		if (segLen <= maxSegmentMm) {
			out.addVertex(b);
			continue;
		}
		const int steps = std::max(1, (int)std::ceil(segLen / maxSegmentMm));
		for (int s = 1; s <= steps; ++s) {
			const float t = (float)s / (float)steps;
			out.addVertex(glm::mix(a, b, t));
		}
	}
	out.setClosed(src.isClosed());
	return out;
}

void appendPolylineJoin(ofPolyline& dst, const ofPolyline& src, bool reverseSrc) {
	ofPolyline s = src;
	if (reverseSrc) reversePolyline(s);
	for (size_t i = 1; i < s.size(); ++i) {
		dst.addVertex(s[i]);
	}
}

void prependPolylineJoin(ofPolyline& dst, const ofPolyline& src, bool reverseSrc) {
	ofPolyline s = src;
	if (reverseSrc) reversePolyline(s);
	std::vector<glm::vec3> merged;
	merged.reserve(s.size() + dst.size());
	for (size_t i = 0; i + 1 < s.size(); ++i) {
		merged.push_back(s[i]);
	}
	for (const auto& v : dst.getVertices()) {
		merged.push_back(v);
	}
	dst.clear();
	for (const auto& v : merged) {
		dst.addVertex(v);
	}
	dst.setClosed(dst.isClosed());
}

void mergePolylinesVpype(std::vector<ofPolyline>& lines, float toleranceMm, bool allowFlip) {
	if (lines.size() < 2) return;

	std::vector<size_t> all;
	all.reserve(lines.size());
	for (size_t i = 0; i < lines.size(); ++i) all.push_back(i);

	LineIndex index(lines, all, true);
	std::vector<ofPolyline> merged;

	while (!index.empty()) {
		size_t firstIdx = 0;
		ofPolyline line;
		if (!index.popFront(firstIdx, line)) break;

		while (true) {
			const glm::vec2 endPt = {line.getVertices().back().x, line.getVertices().back().y};
			const glm::vec2 startPt = {line.getVertices().front().x, line.getVertices().front().y};

			size_t idx = 0;
			bool reverseEnd = false;
			bool progressed = false;

			if (index.findNearestWithin(endPt, toleranceMm, idx, reverseEnd)) {
				if (!reverseEnd || allowFlip) {
					ofPolyline other = lines[idx];
					appendPolylineJoin(line, other, reverseEnd);
					index.pop(idx);
					progressed = true;
				}
			}
			if (progressed) continue;

			if (index.findNearestWithin(startPt, toleranceMm, idx, reverseEnd)) {
				if (reverseEnd || allowFlip) {
					ofPolyline other = lines[idx];
					prependPolylineJoin(line, other, !reverseEnd);
					index.pop(idx);
					progressed = true;
				}
			}
			if (!progressed) break;
		}
		merged.push_back(std::move(line));
	}
	lines = std::move(merged);
}

float penUpLengthMm(const std::vector<ofPolyline>& lines) {
	float travel = 0.f;
	for (size_t i = 1; i < lines.size(); ++i) {
		if (lines[i - 1].size() == 0 || lines[i].size() == 0) continue;
		const auto& a = lines[i - 1].getVertices().back();
		const auto& b = lines[i].getVertices().front();
		travel += glm::length(glm::vec2(b.x - a.x, b.y - a.y));
	}
	return travel;
}

bool sortPolylinesGreedyVpype(std::vector<ofPolyline>& lines, bool allowFlip) {
	if (lines.size() < 2) return false;

	const float originalTravel = penUpLengthMm(lines);
	std::vector<ofPolyline> working = lines;

	std::vector<size_t> rest;
	for (size_t i = 1; i < working.size(); ++i) rest.push_back(i);

	LineIndex index(working, rest, allowFlip);
	std::vector<ofPolyline> sorted;
	sorted.push_back(working[0]);

	while (!index.empty()) {
		const auto& last = sorted.back();
		if (last.size() == 0) break;
		const glm::vec2 cursor = {last.getVertices().back().x, last.getVertices().back().y};

		size_t idx = 0;
		bool reverseEnd = false;
		if (index.findNearest(cursor, idx, reverseEnd)) {
			index.pop(idx);
			ofPolyline next = working[idx];
			if (reverseEnd && allowFlip) reversePolyline(next);
			sorted.push_back(std::move(next));
			continue;
		}

		size_t fallbackIdx = 0;
		ofPolyline fallback;
		if (index.popFront(fallbackIdx, fallback)) {
			sorted.push_back(std::move(fallback));
		} else {
			break;
		}
	}

	const float newTravel = penUpLengthMm(sorted);
	if (newTravel <= originalTravel + 1e-4f) {
		lines = std::move(sorted);
		return newTravel + 1e-4f < originalTravel;
	}
	return false;
}

std::vector<int> layerIdsInOrder(const StrokeDocument& doc) {
	std::vector<int> order;
	std::map<int, bool> seen;
	for (const auto& m : doc.meta) {
		if (!seen[m.layerId]) {
			seen[m.layerId] = true;
			order.push_back(m.layerId);
		}
	}
	return order;
}

std::vector<size_t> pathIndicesForLayer(const StrokeDocument& doc, int layerId) {
	std::vector<size_t> out;
	for (size_t i = 0; i < doc.meta.size(); ++i) {
		if (doc.meta[i].layerId == layerId) out.push_back(i);
	}
	return out;
}

bool isPathClosed(const ofPolyline& pl, float toleranceMm) {
	if (pl.size() < 2) return false;
	const auto& a = pl.getVertices().front();
	const auto& b = pl.getVertices().back();
	return glm::length(glm::vec2(b.x - a.x, b.y - a.y)) <= toleranceMm;
}

ofRectangle boundsForPathIndices(const StrokeDocument& doc, const std::vector<size_t>& indices) {
	ofRectangle r;
	if (indices.empty()) return doc.bounds;
	bool first = true;
	for (size_t i : indices) {
		if (i >= doc.paths.size()) continue;
		for (const auto& v : doc.paths[i].getVertices()) {
			if (first) {
				r.set(v.x, v.y, 0, 0);
				first = false;
			} else {
				r.growToInclude(v.x, v.y);
			}
		}
	}
	return r;
}

bool layerMatchesLayerFilter(int layerId, const ofJson& options) {
	if (!options.contains("layer_ids") || !options["layer_ids"].is_array()) return true;
	const auto& ids = options["layer_ids"];
	if (ids.empty()) return true;
	for (const auto& id : ids) {
		if (id.is_number_integer() && id.get<int>() == layerId) return true;
	}
	return false;
}

bool pathMatchesLayerFilter(const StrokeMeta& meta, const ofJson& options) {
	return layerMatchesLayerFilter(meta.layerId, options);
}

glm::vec2 resolveTransformOrigin(const StrokeDocument& doc, const ofJson& options) {
	std::vector<size_t> affected;
	for (size_t i = 0; i < doc.paths.size(); ++i) {
		if (i < doc.meta.size() && !pathMatchesLayerFilter(doc.meta[i], options)) continue;
		if (doc.paths[i].size() > 0) affected.push_back(i);
	}
	if (options.contains("origin_x_mm") && options.contains("origin_y_mm")) {
		return {options.value("origin_x_mm", 0.0f), options.value("origin_y_mm", 0.0f)};
	}
	const ofRectangle r = boundsForPathIndices(doc, affected);
	if (r.width <= 0 && r.height <= 0) return {r.x, r.y};
	return {r.x + r.width * 0.5f, r.y + r.height * 0.5f};
}

void scaleVerticesAtOrigin(ofPolyline& pl, const glm::vec2& origin, float sx, float sy) {
	for (auto& v : pl.getVertices()) {
		v.x = origin.x + sx * (v.x - origin.x);
		v.y = origin.y + sy * (v.y - origin.y);
	}
}

void rotateVerticesClockwise(ofPolyline& pl, const glm::vec2& origin, float angleDeg) {
	const float rad = angleDeg * static_cast<float>(M_PI) / 180.f;
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	for (auto& v : pl.getVertices()) {
		const float dx = v.x - origin.x;
		const float dy = v.y - origin.y;
		v.x = origin.x + c * dx - s * dy;
		v.y = origin.y + s * dx + c * dy;
	}
}

void skewVerticesAtOrigin(ofPolyline& pl, const glm::vec2& origin, float skewXDeg, float skewYDeg) {
	const float tx = std::tan(skewXDeg * static_cast<float>(M_PI) / 180.f);
	const float ty = std::tan(skewYDeg * static_cast<float>(M_PI) / 180.f);
	for (auto& v : pl.getVertices()) {
		const float dx = v.x - origin.x;
		const float dy = v.y - origin.y;
		v.x = origin.x + dx + tx * dy;
		v.y = origin.y + dy + ty * dx;
	}
}

bool pagePresetSizeMm(const std::string& preset, float& widthMm, float& heightMm) {
	static const std::map<std::string, glm::vec2> kPresets = {
		{"a6", {105.f, 148.f}},
		{"a5", {148.f, 210.f}},
		{"a4", {210.f, 297.f}},
		{"a3", {297.f, 420.f}},
		{"a2", {420.f, 594.f}},
		{"a1", {594.f, 841.f}},
		{"a0", {841.f, 1189.f}},
		{"letter", {215.9f, 279.4f}},
		{"legal", {215.9f, 355.6f}},
		{"executive", {184.15f, 266.7f}},
		{"tabloid", {279.4f, 431.8f}},
	};
	auto it = kPresets.find(preset);
	if (it == kPresets.end()) return false;
	widthMm = it->second.x;
	heightMm = it->second.y;
	return true;
}

} // namespace plotproc
