#pragma once

#include "ofMain.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace plotproc {

/// Spatial index for stroke endpoints — port of vpype ``LineIndex`` (scipy KDTree → uniform grid).
class LineIndex {
public:
	/// @param paths  Source polylines (must outlive this index).
	/// @param indices  Subset of path indices to index (empty = all paths).
	/// @param reverse  Index line ends as well as starts (for merge / sort with flip).
	LineIndex(const std::vector<ofPolyline>& paths,
	          const std::vector<size_t>& indices,
	          bool reverse = true);

	size_t size() const;
	bool empty() const { return size() == 0; }

	/// First available path index (vpype ``pop_front``).
	bool popFront(size_t& pathIndex, ofPolyline& out);

	/// Mark path unavailable; returns false if already removed.
	bool pop(size_t pathIndex);

	/// Nearest start (or end if reverse) among available paths.
	bool findNearest(const glm::vec2& p, size_t& pathIndex, bool& reverseEnd) const;

	/// Nearest within @p maxDist (vpype ``find_nearest_within``).
	bool findNearestWithin(const glm::vec2& p,
	                       float maxDist,
	                       size_t& pathIndex,
	                       bool& reverseEnd) const;

private:
	struct Entry {
		size_t pathIndex = 0;
		bool isEnd = false; // false = start, true = end (when reverse indexing)
	};

	void buildGrid(float cellSize);
	void insertEntry(size_t pathIndex, bool isEnd);
	uint64_t cellKey(int cx, int cy) const;

	const std::vector<ofPolyline>* m_paths = nullptr;
	std::vector<bool> m_available;
	bool m_reverse = true;
	float m_cellSize = 1.f;

	std::vector<Entry> m_entries;
	std::unordered_map<uint64_t, std::vector<size_t>> m_grid;
};

} // namespace plotproc
