#include "CropProcessor.h"
#include "../PlotMetrics.h"
#include <algorithm>

namespace plotproc {

namespace {

glm::vec2 clipSegmentAxis(float v0, float v1, float lo, float hi, glm::vec2 a, glm::vec2 b) {
	if (v0 >= lo && v0 <= hi) return a;
	if (v1 == v0) return a;
	const float t = (v0 < lo) ? (lo - v0) / (v1 - v0) : (hi - v0) / (v1 - v0);
	return glm::mix(a, b, std::clamp(t, 0.f, 1.f));
}

bool clipSegmentRect(glm::vec2 a, glm::vec2 b, float x0, float y0, float x1, float y1,
                     glm::vec2& outA, glm::vec2& outB) {
	float ax = a.x, ay = a.y, bx = b.x, by = b.y;
	if (ax < x0) a = clipSegmentAxis(ax, bx, x0, x1, a, b), ax = a.x;
	if (bx < x0) b = clipSegmentAxis(bx, ax, x0, x1, b, a), bx = b.x;
	if (ax > x1) a = clipSegmentAxis(ax, bx, x1, x0, a, b), ax = a.x;
	if (bx > x1) b = clipSegmentAxis(bx, ax, x1, x0, b, a), bx = b.x;
	if (ay < y0) a = clipSegmentAxis(ay, by, y0, y1, a, b), ay = a.y;
	if (by < y0) b = clipSegmentAxis(by, ay, y0, y1, b, a), by = b.y;
	if (ay > y1) a = clipSegmentAxis(ay, by, y1, y0, a, b), ay = a.y;
	if (by > y1) b = clipSegmentAxis(by, ay, y1, y0, b, a), by = b.y;

	const bool aIn = ax >= x0 && ax <= x1 && ay >= y0 && ay <= y1;
	const bool bIn = bx >= x0 && bx <= x1 && by >= y0 && by <= y1;
	if (!aIn && !bIn) return false;
	outA = a;
	outB = b;
	return true;
}

std::vector<ofPolyline> cropPolyline(const ofPolyline& pl, float x, float y, float w, float h) {
	std::vector<ofPolyline> out;
	if (pl.size() < 2) return out;
	const float x0 = x, y0 = y, x1 = x + w, y1 = y + h;

	ofPolyline current;
	for (size_t i = 1; i < pl.size(); ++i) {
		glm::vec2 a = {pl[i - 1].x, pl[i - 1].y};
		glm::vec2 b = {pl[i].x, pl[i].y};
		glm::vec2 ca, cb;
		if (!clipSegmentRect(a, b, x0, y0, x1, y1, ca, cb)) {
			if (current.size() >= 2) out.push_back(current);
			current.clear();
			continue;
		}
		if (current.size() == 0) {
			current.addVertex(ca.x, ca.y);
		} else {
			const auto& last = current.getVertices().back();
			if (glm::length(glm::vec2(last.x - ca.x, last.y - ca.y)) > 1e-4f) {
				if (current.size() >= 2) out.push_back(current);
				current.clear();
				current.addVertex(ca.x, ca.y);
			}
		}
		current.addVertex(cb.x, cb.y);
	}
	if (current.size() >= 2) out.push_back(current);
	return out;
}

} // namespace

ofJson CropProcessor::defaultOptions() const {
	return {
		{"x_mm", 0.0},
		{"y_mm", 0.0},
		{"width_mm", 100.0},
		{"height_mm", 100.0}
	};
}

void CropProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const float x = options.value("x_mm", 0.0f);
	const float y = options.value("y_mm", 0.0f);
	const float w = options.value("width_mm", 100.0f);
	const float h = options.value("height_mm", 100.0f);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.syncMetaSize();
	std::vector<ofPolyline> newPaths;
	std::vector<StrokeMeta> newMeta;

	for (size_t i = 0; i < doc.paths.size(); ++i) {
		auto parts = cropPolyline(doc.paths[i], x, y, w, h);
		for (auto& p : parts) {
			newPaths.push_back(std::move(p));
			newMeta.push_back(doc.meta[i]);
		}
	}

	doc.paths = std::move(newPaths);
	doc.meta = std::move(newMeta);
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Cropped to rect";
	}
}

} // namespace plotproc
