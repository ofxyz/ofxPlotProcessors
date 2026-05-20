#include "ProcessorUtils.h"
#include "../LineIndex.h"
#include <algorithm>
#include <cmath>
#include <map>

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

} // namespace plotproc
