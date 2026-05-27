#include "LineIndex.h"
#include "processors/ProcessorUtils.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace plotproc {

namespace {

glm::vec2 pathPoint(const std::vector<ofPath>& paths, size_t idx, bool atEnd) {
	if (idx >= paths.size()) return {};
	return atEnd ? pathEndPt(paths[idx]) : pathStartPt(paths[idx]);
}

float dist2(const glm::vec2& a, const glm::vec2& b) {
	return glm::length2(b - a);
}

} // namespace

LineIndex::LineIndex(const std::vector<ofPath>& paths,
                     const std::vector<size_t>& indices,
                     bool reverse)
	: m_paths(&paths)
	, m_reverse(reverse) {
	std::vector<size_t> idxs = indices;
	if (idxs.empty()) {
		idxs.resize(paths.size());
		for (size_t i = 0; i < paths.size(); ++i) idxs[i] = i;
	}

	m_available.assign(paths.size(), false);
	for (size_t i : idxs) {
		if (i < paths.size() && !pathIsEmpty(paths[i]))
			m_available[i] = true;
	}

	float maxSpan = 1.f;
	for (size_t i = 0; i < paths.size(); ++i) {
		if (!m_available[i]) continue;
		// Use command points to compute span (fast, no tessellation)
		for (const auto& cmd : paths[i].getCommands()) {
			maxSpan = std::max(maxSpan, std::abs(cmd.to.x));
			maxSpan = std::max(maxSpan, std::abs(cmd.to.y));
		}
	}
	m_cellSize = std::max(0.5f, maxSpan / 128.f);
	buildGrid(m_cellSize);
}

void LineIndex::buildGrid(float cellSize) {
	m_cellSize = std::max(1e-4f, cellSize);
	m_entries.clear();
	m_grid.clear();

	for (size_t pi = 0; pi < m_paths->size(); ++pi) {
		if (!m_available[pi]) continue;
		if (pathIsEmpty((*m_paths)[pi])) continue;
		insertEntry(pi, false);
		if (m_reverse) insertEntry(pi, true);
	}
}

uint64_t LineIndex::cellKey(int cx, int cy) const {
	return (uint64_t)(uint32_t)cx << 32 | (uint32_t)cy;
}

void LineIndex::insertEntry(size_t pathIndex, bool isEnd) {
	const glm::vec2 p = pathPoint(*m_paths, pathIndex, isEnd);
	const int cx = (int)std::floor(p.x / m_cellSize);
	const int cy = (int)std::floor(p.y / m_cellSize);
	const size_t ei = m_entries.size();
	m_entries.push_back({pathIndex, isEnd});
	m_grid[cellKey(cx, cy)].push_back(ei);
}

size_t LineIndex::size() const {
	return (size_t)std::count(m_available.begin(), m_available.end(), true);
}

bool LineIndex::pop(size_t pathIndex) {
	if (pathIndex >= m_available.size() || !m_available[pathIndex]) return false;
	m_available[pathIndex] = false;
	return true;
}

bool LineIndex::popFront(size_t& pathIndex, ofPath& out) {
	for (size_t i = 0; i < m_available.size(); ++i) {
		if (!m_available[i]) continue;
		if (pathIsEmpty((*m_paths)[i])) continue;
		pathIndex = i;
		m_available[i] = false;
		out = (*m_paths)[i];
		return true;
	}
	return false;
}

bool LineIndex::findNearest(const glm::vec2& p, size_t& pathIndex, bool& reverseEnd) const {
	const float maxSearch = m_cellSize * 512.f;
	return findNearestWithin(p, maxSearch, pathIndex, reverseEnd);
}

bool LineIndex::findNearestWithin(const glm::vec2& p,
                                  float maxDist,
                                  size_t& pathIndex,
                                  bool& reverseEnd) const {
	const float maxDist2 = maxDist * maxDist;
	float best = maxDist2;
	bool found = false;
	pathIndex  = 0;
	reverseEnd = false;

	const int cx0 = (int)std::floor(p.x / m_cellSize);
	const int cy0 = (int)std::floor(p.y / m_cellSize);
	const int rings = std::max(1, (int)std::ceil(maxDist / m_cellSize) + 1);

	for (int ring = 0; ring <= rings; ++ring) {
		for (int dx = -ring; dx <= ring; ++dx) {
			for (int dy = -ring; dy <= ring; ++dy) {
				if (ring > 0 && std::abs(dx) != ring && std::abs(dy) != ring) continue;
				auto it = m_grid.find(cellKey(cx0 + dx, cy0 + dy));
				if (it == m_grid.end()) continue;
				for (size_t ei : it->second) {
					const Entry& e = m_entries[ei];
					if (!m_available[e.pathIndex]) continue;
					const glm::vec2 q = pathPoint(*m_paths, e.pathIndex, e.isEnd);
					const float d2 = dist2(p, q);
					if (d2 <= best) {
						best      = d2;
						pathIndex = e.pathIndex;
						reverseEnd = e.isEnd;
						found     = true;
					}
				}
			}
		}
		if (found && ring > 0 &&
		    best < (float)((ring - 1) * m_cellSize) * (float)((ring - 1) * m_cellSize)) {
			break;
		}
	}
	return found;
}

} // namespace plotproc
