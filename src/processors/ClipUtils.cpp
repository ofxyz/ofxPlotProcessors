#include "ClipUtils.h"
#include <algorithm>
#include <cmath>

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

bool insideCircle(glm::vec2 p, const glm::vec2& c, float r) {
	return glm::length(p - c) <= r + 1e-5f;
}

/// Intersection parameters t in [0,1] where |a + t(b-a) - c| = r (0, 1, or 2 values).
std::vector<float> segmentCircleCrossings(glm::vec2 a, glm::vec2 b, const glm::vec2& c, float r) {
	const glm::vec2 d = b - a;
	const glm::vec2 f = a - c;
	const float A = glm::dot(d, d);
	const float B = 2.f * glm::dot(f, d);
	const float C = glm::dot(f, f) - r * r;
	std::vector<float> ts;
	if (A < 1e-12f) return ts;
	const float disc = B * B - 4.f * A * C;
	if (disc < 0.f) return ts;
	const float sdisc = std::sqrt(disc);
	const float t0 = (-B - sdisc) / (2.f * A);
	const float t1 = (-B + sdisc) / (2.f * A);
	for (float t : {t0, t1}) {
		if (t >= -1e-5f && t <= 1.f + 1e-5f) {
			ts.push_back(std::clamp(t, 0.f, 1.f));
		}
	}
	std::sort(ts.begin(), ts.end());
	ts.erase(std::unique(ts.begin(), ts.end(), [](float u, float v) { return std::abs(u - v) < 1e-5f; }), ts.end());
	return ts;
}

glm::vec2 lerpPt(glm::vec2 a, glm::vec2 b, float t) {
	return glm::mix(a, b, t);
}

void appendVertex(ofPolyline& pl, glm::vec2 p) {
	if (pl.size() > 0) {
		const auto& last = pl.getVertices().back();
		if (glm::length(glm::vec2(last.x - p.x, last.y - p.y)) < 1e-5f) return;
	}
	pl.addVertex(p.x, p.y);
}

} // namespace

std::vector<ofPolyline> cropPolylineToRect(const ofPolyline& pl, float x, float y, float w, float h) {
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
			appendVertex(current, ca);
		} else {
			const auto& last = current.getVertices().back();
			if (glm::length(glm::vec2(last.x - ca.x, last.y - ca.y)) > 1e-4f) {
				if (current.size() >= 2) out.push_back(current);
				current.clear();
				appendVertex(current, ca);
			}
		}
		appendVertex(current, cb);
	}
	if (current.size() >= 2) out.push_back(current);
	return out;
}

std::vector<ofPolyline> cropPolylineToCircle(const ofPolyline& pl, const glm::vec2& center, float radiusMm) {
	std::vector<ofPolyline> out;
	if (pl.size() < 2 || radiusMm <= 0.f) return out;

	ofPolyline current;
	for (size_t i = 1; i < pl.size(); ++i) {
		glm::vec2 a = {pl[i - 1].x, pl[i - 1].y};
		glm::vec2 b = {pl[i].x, pl[i].y};
		const bool ia = insideCircle(a, center, radiusMm);
		const bool ib = insideCircle(b, center, radiusMm);
		auto ts = segmentCircleCrossings(a, b, center, radiusMm);

		auto emitChord = [&](glm::vec2 p0, glm::vec2 p1) {
			if (current.size() >= 2) out.push_back(current);
			current.clear();
			appendVertex(current, p0);
			appendVertex(current, p1);
			if (current.size() >= 2) out.push_back(current);
			current.clear();
		};

		if (ia && ib) {
			if (current.size() == 0) appendVertex(current, a);
			appendVertex(current, b);
		} else if (ia && !ib) {
			if (current.size() == 0) appendVertex(current, a);
			if (!ts.empty()) appendVertex(current, lerpPt(a, b, ts.back()));
			if (current.size() >= 2) out.push_back(current);
			current.clear();
		} else if (!ia && ib) {
			if (!ts.empty()) appendVertex(current, lerpPt(a, b, ts.front()));
			appendVertex(current, b);
		} else {
			if (ts.size() >= 2) {
				emitChord(lerpPt(a, b, ts.front()), lerpPt(a, b, ts.back()));
			}
		}
	}
	if (current.size() >= 2) out.push_back(current);
	return out;
}

} // namespace plotproc
