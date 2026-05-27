#include "ProcessorUtils.h"
#include "../LineIndex.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <string>

namespace plotproc {

// ============================================================
//  Internal polyline helpers (used by tessellate-based ops)
// ============================================================

static void reversePolyline(ofPolyline& pl) {
	if (pl.size() < 2) return;
	std::vector<glm::vec3> verts = pl.getVertices();
	std::reverse(verts.begin(), verts.end());
	pl.clear();
	for (const auto& v : verts) {
		pl.addVertex(v);
	}
	pl.setClosed(pl.isClosed());
}

static void appendPolylineJoin(ofPolyline& dst, const ofPolyline& src, bool reverseSrc) {
	ofPolyline s = src;
	if (reverseSrc) reversePolyline(s);
	for (size_t i = 1; i < s.size(); ++i) {
		dst.addVertex(s[i]);
	}
}

static void prependPolylineJoin(ofPolyline& dst, const ofPolyline& src, bool reverseSrc) {
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

// ============================================================
//  ofPath helpers
// ============================================================

glm::vec2 pathStartPt(const ofPath& p) {
	for (const auto& cmd : p.getCommands()) {
		if (cmd.type != ofPath::Command::close) {
			return {cmd.to.x, cmd.to.y};
		}
	}
	return {};
}

glm::vec2 pathEndPt(const ofPath& p) {
	const auto& cmds = p.getCommands();
	for (int i = (int)cmds.size() - 1; i >= 0; --i) {
		if (cmds[i].type != ofPath::Command::close) {
			return {cmds[i].to.x, cmds[i].to.y};
		}
	}
	return {};
}

bool pathIsEmpty(const ofPath& p) {
	for (const auto& cmd : p.getCommands()) {
		switch (cmd.type) {
			case ofPath::Command::lineTo:
			case ofPath::Command::curveTo:
			case ofPath::Command::bezierTo:
			case ofPath::Command::quadBezierTo:
			case ofPath::Command::arc:
			case ofPath::Command::arcNegative:
				return false;
			default:
				break;
		}
	}
	return true;
}

ofPolyline pathToPolyline(const ofPath& p) {
	const auto& outlines = p.getOutline();
	if (outlines.empty()) return {};
	return outlines[0];
}

ofPath polylineToPath(const ofPolyline& pl) {
	ofPath p;
	if (pl.size() < 1) return p;
	p.moveTo(pl[0]);
	for (size_t i = 1; i < pl.size(); ++i) {
		p.lineTo(pl[i]);
	}
	if (pl.isClosed()) p.close();
	return p;
}

// ---- Rebuild a path from a modified command list --------------------------

static bool hasArcCommands(const ofPath& p) {
	for (const auto& cmd : p.getCommands()) {
		if (cmd.type == ofPath::Command::arc ||
		    cmd.type == ofPath::Command::arcNegative) {
			return true;
		}
	}
	return false;
}

static void rebuildPathFromCommands(ofPath& p,
                                    const std::vector<ofPath::Command>& cmds) {
	p.clear();
	for (const auto& cmd : cmds) {
		switch (cmd.type) {
			case ofPath::Command::moveTo:      p.moveTo(cmd.to); break;
			case ofPath::Command::lineTo:      p.lineTo(cmd.to); break;
			case ofPath::Command::curveTo:     p.curveTo(cmd.to); break;
			case ofPath::Command::bezierTo:    p.bezierTo(cmd.cp1, cmd.cp2, cmd.to); break;
			case ofPath::Command::quadBezierTo:p.quadBezierTo(cmd.cp1, cmd.to); break;
			case ofPath::Command::arc:
				p.arc(cmd.to.x, cmd.to.y,
				      cmd.cp1.x, cmd.cp1.y,
				      cmd.cp2.x, cmd.cp2.y, true);
				break;
			case ofPath::Command::arcNegative:
				p.arc(cmd.to.x, cmd.to.y,
				      cmd.cp1.x, cmd.cp1.y,
				      cmd.cp2.x, cmd.cp2.y, false);
				break;
			case ofPath::Command::close:       p.close(); break;
			default: break;
		}
	}
}

static void appendCommandsToPath(ofPath& dst,
                                  const std::vector<ofPath::Command>& cmds,
                                  bool skipLeadingMoveTo) {
	bool skipped = !skipLeadingMoveTo;
	for (const auto& cmd : cmds) {
		if (!skipped) {
			skipped = true;
			if (cmd.type == ofPath::Command::moveTo) continue;
		}
		switch (cmd.type) {
			case ofPath::Command::moveTo:       dst.moveTo(cmd.to); break;
			case ofPath::Command::lineTo:       dst.lineTo(cmd.to); break;
			case ofPath::Command::curveTo:      dst.curveTo(cmd.to); break;
			case ofPath::Command::bezierTo:     dst.bezierTo(cmd.cp1, cmd.cp2, cmd.to); break;
			case ofPath::Command::quadBezierTo: dst.quadBezierTo(cmd.cp1, cmd.to); break;
			case ofPath::Command::arc:
				dst.arc(cmd.to.x, cmd.to.y,
				        cmd.cp1.x, cmd.cp1.y,
				        cmd.cp2.x, cmd.cp2.y, true);
				break;
			case ofPath::Command::arcNegative:
				dst.arc(cmd.to.x, cmd.to.y,
				        cmd.cp1.x, cmd.cp1.y,
				        cmd.cp2.x, cmd.cp2.y, false);
				break;
			case ofPath::Command::close:        dst.close(); break;
			default: break;
		}
	}
}

// ============================================================
//  transformPathInPlace
// ============================================================

void transformPathInPlace(ofPath& p,
                          const std::function<glm::vec2(const glm::vec2&)>& fn) {
	if (hasArcCommands(p)) {
		// For arc commands fall back to tessellation
		ofPolyline pl = pathToPolyline(p);
		for (auto& v : pl.getVertices()) {
			const glm::vec2 t = fn({v.x, v.y});
			v.x = t.x;
			v.y = t.y;
		}
		p = polylineToPath(pl);
		return;
	}

	std::vector<ofPath::Command> cmds = p.getCommands(); // copy

	auto tf = [&](glm::vec3& pt) {
		const glm::vec2 t = fn({pt.x, pt.y});
		pt.x = t.x;
		pt.y = t.y;
	};

	for (auto& cmd : cmds) {
		switch (cmd.type) {
			case ofPath::Command::moveTo:
			case ofPath::Command::lineTo:
			case ofPath::Command::curveTo:
				tf(cmd.to);
				break;
			case ofPath::Command::bezierTo:
				tf(cmd.cp1);
				tf(cmd.cp2);
				tf(cmd.to);
				break;
			case ofPath::Command::quadBezierTo:
				tf(cmd.cp1);
				tf(cmd.to);
				break;
			default:
				break;
		}
	}
	rebuildPathFromCommands(p, cmds);
}

// ============================================================
//  reversePath
// ============================================================

void reversePath(ofPath& p) {
	ofPolyline pl = pathToPolyline(p);
	reversePolyline(pl);
	p = polylineToPath(pl);
}

// ============================================================
//  appendPathJoin / prependPathJoin
// ============================================================

void appendPathJoin(ofPath& dst, const ofPath& src, bool reverseSrc) {
	if (reverseSrc) {
		// Reversed: tessellate src, reverse, then add as lines
		ofPolyline pl = pathToPolyline(src);
		reversePolyline(pl);
		for (size_t i = 1; i < pl.size(); ++i) {
			dst.lineTo(pl[i]);
		}
	} else {
		// Forward: copy commands directly to preserve bezier curves
		appendCommandsToPath(dst, src.getCommands(), /*skipLeadingMoveTo=*/true);
	}
}

void prependPathJoin(ofPath& dst, const ofPath& src, bool reverseSrc) {
	// Save existing dst commands before we clear it
	const std::vector<ofPath::Command> dstCmds = dst.getCommands();
	dst.clear();

	if (reverseSrc) {
		ofPolyline pl = pathToPolyline(src);
		reversePolyline(pl);
		if (!pl.empty()) {
			dst.moveTo(pl[0]);
			for (size_t i = 1; i < pl.size(); ++i) {
				dst.lineTo(pl[i]);
			}
		}
	} else {
		// Copy src commands with original structure
		appendCommandsToPath(dst, src.getCommands(), /*skipLeadingMoveTo=*/false);
	}

	// Append dst commands (skip its leading moveTo since src covers that point)
	appendCommandsToPath(dst, dstCmds, /*skipLeadingMoveTo=*/true);
}

// ============================================================
//  mergePathsVpype
// ============================================================

static float penUpLengthMm(const std::vector<ofPath>& paths) {
	float travel = 0.f;
	for (size_t i = 1; i < paths.size(); ++i) {
		if (pathIsEmpty(paths[i - 1]) || pathIsEmpty(paths[i])) continue;
		const glm::vec2 a = pathEndPt(paths[i - 1]);
		const glm::vec2 b = pathStartPt(paths[i]);
		travel += glm::length(b - a);
	}
	return travel;
}

void mergePathsVpype(std::vector<ofPath>& paths, float toleranceMm, bool allowFlip) {
	if (paths.size() < 2) return;

	std::vector<size_t> all;
	all.reserve(paths.size());
	for (size_t i = 0; i < paths.size(); ++i) all.push_back(i);

	LineIndex index(paths, all, true);
	std::vector<ofPath> merged;

	while (!index.empty()) {
		size_t firstIdx = 0;
		ofPath line;
		if (!index.popFront(firstIdx, line)) break;

		while (true) {
			const glm::vec2 endPt   = pathEndPt(line);
			const glm::vec2 startPt = pathStartPt(line);

			size_t idx = 0;
			bool reverseEnd  = false;
			bool progressed  = false;

			if (index.findNearestWithin(endPt, toleranceMm, idx, reverseEnd)) {
				if (!reverseEnd || allowFlip) {
					appendPathJoin(line, paths[idx], reverseEnd);
					index.pop(idx);
					progressed = true;
				}
			}
			if (progressed) continue;

			if (index.findNearestWithin(startPt, toleranceMm, idx, reverseEnd)) {
				if (reverseEnd || allowFlip) {
					prependPathJoin(line, paths[idx], !reverseEnd);
					index.pop(idx);
					progressed = true;
				}
			}
			if (!progressed) break;
		}
		merged.push_back(std::move(line));
	}
	paths = std::move(merged);
}

// ============================================================
//  sortPathsGreedyVpype
// ============================================================

bool sortPathsGreedyVpype(std::vector<ofPath>& paths, bool allowFlip) {
	if (paths.size() < 2) return false;

	const float originalTravel = penUpLengthMm(paths);
	std::vector<ofPath> working = paths;

	std::vector<size_t> rest;
	for (size_t i = 1; i < working.size(); ++i) rest.push_back(i);

	LineIndex index(working, rest, allowFlip);
	std::vector<ofPath> sorted;
	sorted.push_back(working[0]);

	while (!index.empty()) {
		const ofPath& last   = sorted.back();
		if (pathIsEmpty(last)) break;
		const glm::vec2 cursor = pathEndPt(last);

		size_t idx       = 0;
		bool reverseEnd  = false;
		if (index.findNearest(cursor, idx, reverseEnd)) {
			index.pop(idx);
			ofPath next = working[idx];
			if (reverseEnd && allowFlip) reversePath(next);
			sorted.push_back(std::move(next));
			continue;
		}

		size_t fallbackIdx = 0;
		ofPath fallback;
		if (index.popFront(fallbackIdx, fallback)) {
			sorted.push_back(std::move(fallback));
		} else {
			break;
		}
	}

	const float newTravel = penUpLengthMm(sorted);
	if (newTravel <= originalTravel + 1e-4f) {
		paths = std::move(sorted);
		return newTravel + 1e-4f < originalTravel;
	}
	return false;
}

// ============================================================
//  Layer / filter helpers
// ============================================================

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

bool isPathClosed(const ofPath& p, float toleranceMm) {
	if (p.isClosed()) return true;
	const glm::vec2 start = pathStartPt(p);
	const glm::vec2 end   = pathEndPt(p);
	return glm::length(end - start) <= toleranceMm;
}

ofRectangle boundsForPathIndices(const StrokeDocument& doc,
                                  const std::vector<size_t>& indices) {
	ofRectangle r;
	if (indices.empty()) return doc.bounds;
	bool first = true;
	for (size_t i : indices) {
		if (i >= doc.paths.size()) continue;
		for (const auto& cmd : doc.paths[i].getCommands()) {
			if (cmd.type == ofPath::Command::close) continue;
			if (first) {
				r.set(cmd.to.x, cmd.to.y, 0, 0);
				first = false;
			} else {
				r.growToInclude(cmd.to.x, cmd.to.y);
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
		if (!pathIsEmpty(doc.paths[i])) affected.push_back(i);
	}
	if (options.contains("origin_x_mm") && options.contains("origin_y_mm")) {
		return {options.value("origin_x_mm", 0.0f), options.value("origin_y_mm", 0.0f)};
	}
	const ofRectangle r = boundsForPathIndices(doc, affected);
	if (r.width <= 0 && r.height <= 0) return {r.x, r.y};
	return {r.x + r.width * 0.5f, r.y + r.height * 0.5f};
}

// ============================================================
//  Affine path transforms
// ============================================================

void scalePathAtOrigin(ofPath& p, const glm::vec2& origin, float sx, float sy) {
	transformPathInPlace(p, [&](const glm::vec2& v) {
		return glm::vec2{origin.x + sx * (v.x - origin.x),
		                 origin.y + sy * (v.y - origin.y)};
	});
}

void rotatePathClockwise(ofPath& p, const glm::vec2& origin, float angleDeg) {
	const float rad = angleDeg * static_cast<float>(M_PI) / 180.f;
	const float c   = std::cos(rad);
	const float s   = std::sin(rad);
	transformPathInPlace(p, [&](const glm::vec2& v) {
		const float dx = v.x - origin.x;
		const float dy = v.y - origin.y;
		return glm::vec2{origin.x + c * dx - s * dy,
		                 origin.y + s * dx + c * dy};
	});
}

void skewPathAtOrigin(ofPath& p, const glm::vec2& origin,
                      float skewXDeg, float skewYDeg) {
	const float tx = std::tan(skewXDeg * static_cast<float>(M_PI) / 180.f);
	const float ty = std::tan(skewYDeg * static_cast<float>(M_PI) / 180.f);
	transformPathInPlace(p, [&](const glm::vec2& v) {
		const float dx = v.x - origin.x;
		const float dy = v.y - origin.y;
		return glm::vec2{origin.x + dx + tx * dy,
		                 origin.y + dy + ty * dx};
	});
}

// ============================================================
//  Polyline resampling
// ============================================================

ofPolyline resamplePolyline(const ofPolyline& src, float maxSegmentMm) {
	ofPolyline out;
	if (src.size() < 2 || maxSegmentMm <= 0.f) return src;
	const auto& verts = src.getVertices();
	out.addVertex(verts.front());
	for (size_t i = 1; i < verts.size(); ++i) {
		const glm::vec3 a = verts[i - 1];
		const glm::vec3 b = verts[i];
		const float segLen = glm::length(glm::vec2(b.x - a.x, b.y - a.y));
		if (segLen <= maxSegmentMm) {
			out.addVertex(b);
			continue;
		}
		const int steps = std::max(1, (int)std::ceil(segLen / maxSegmentMm));
		for (int step = 1; step <= steps; ++step) {
			out.addVertex(glm::mix(a, b, (float)step / (float)steps));
		}
	}
	out.setClosed(src.isClosed());
	return out;
}

// ============================================================
//  Page presets
// ============================================================

bool pagePresetSizeMm(const std::string& preset, float& widthMm, float& heightMm) {
	static const std::map<std::string, glm::vec2> kPresets = {
		{"a6",        {105.f,   148.f}},
		{"a5",        {148.f,   210.f}},
		{"a4",        {210.f,   297.f}},
		{"a3",        {297.f,   420.f}},
		{"a2",        {420.f,   594.f}},
		{"a1",        {594.f,   841.f}},
		{"a0",        {841.f,  1189.f}},
		{"letter",    {215.9f,  279.4f}},
		{"legal",     {215.9f,  355.6f}},
		{"executive", {184.15f, 266.7f}},
		{"tabloid",   {279.4f,  431.8f}},
	};
	auto it = kPresets.find(preset);
	if (it == kPresets.end()) return false;
	widthMm  = it->second.x;
	heightMm = it->second.y;
	return true;
}

} // namespace plotproc
