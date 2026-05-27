#include "StrokeDocument.h"
#include <limits>

namespace plotproc {

void StrokeDocument::syncMetaSize() {
	if (meta.size() < paths.size()) {
		meta.resize(paths.size());
	} else if (meta.size() > paths.size()) {
		meta.resize(paths.size());
	}
	for (size_t i = 0; i < paths.size(); ++i) {
		meta[i].closed = paths[i].isClosed();
	}
}

void StrokeDocument::rebuildBounds() {
	if (paths.empty()) {
		bounds = {};
		return;
	}
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	auto expand = [&](float x, float y) {
		minX = std::min(minX, x);
		minY = std::min(minY, y);
		maxX = std::max(maxX, x);
		maxY = std::max(maxY, y);
	};

	for (const auto& p : paths) {
		for (const auto& cmd : p.getCommands()) {
			if (cmd.type == ofPath::Command::close) continue;
			expand(cmd.to.x, cmd.to.y);
			if (cmd.type == ofPath::Command::bezierTo) {
				expand(cmd.cp1.x, cmd.cp1.y);
				expand(cmd.cp2.x, cmd.cp2.y);
			} else if (cmd.type == ofPath::Command::quadBezierTo) {
				expand(cmd.cp1.x, cmd.cp1.y);
			}
		}
	}

	if (minX > maxX) {
		bounds = {};
	} else {
		bounds.set(minX, minY, maxX - minX, maxY - minY);
	}
}

size_t StrokeDocument::vertexCount() const {
	size_t n = 0;
	for (const auto& p : paths) {
		for (const auto& ol : p.getOutline()) {
			n += ol.size();
		}
	}
	return n;
}

float StrokeDocument::pathLengthMM(size_t index) const {
	if (index >= paths.size()) return 0.f;
	float len = 0.f;
	for (const auto& ol : paths[index].getOutline()) {
		len += ol.getPerimeter();
	}
	return len;
}

glm::vec2 StrokeDocument::pathStart(size_t index) const {
	if (index >= paths.size()) return {};
	for (const auto& cmd : paths[index].getCommands()) {
		if (cmd.type != ofPath::Command::close) {
			return {cmd.to.x, cmd.to.y};
		}
	}
	return {};
}

glm::vec2 StrokeDocument::pathEnd(size_t index) const {
	if (index >= paths.size()) return {};
	const auto& cmds = paths[index].getCommands();
	for (int j = (int)cmds.size() - 1; j >= 0; --j) {
		if (cmds[j].type != ofPath::Command::close) {
			return {cmds[j].to.x, cmds[j].to.y};
		}
	}
	return {};
}

} // namespace plotproc
