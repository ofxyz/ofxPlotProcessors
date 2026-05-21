#include "TextProcessor.h"
#include "../PlotMetrics.h"
#include "ofTrueTypeFont.h"
#include <algorithm>

namespace plotproc {

namespace {

ofPolyline pathToPolyline(const ofPath& path) {
	ofPolyline pl;
	for (const auto& v : path.getOutline()) {
		pl.addVertex(v.x, v.y, v.z);
	}
	return pl;
}

float textAlignOffsetX(const std::vector<ofPolyline>& lines, const std::string& align) {
	if (lines.empty() || align == "left") return 0.f;
	float minX = 1e9f, maxX = -1e9f;
	for (const auto& pl : lines) {
		for (const auto& v : pl.getVertices()) {
			minX = std::min(minX, v.x);
			maxX = std::max(maxX, v.x);
		}
	}
	if (align == "center") return -(minX + maxX) * 0.5f;
	if (align == "right") return -maxX;
	return -minX;
}

} // namespace

ofJson TextProcessor::defaultOptions() const {
	return {
		{"text", ""},
		{"font_path", ""},
		{"font_size", 18},
		{"x_mm", 0.0},
		{"y_mm", 0.0},
		{"align", "left"},
		{"layer_id", 1},
		{"append", true}
	};
}

void TextProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	const std::string text = options.value("text", std::string(""));
	const std::string fontPath = options.value("font_path", std::string(""));
	const int fontSize = std::max(4, options.value("font_size", 18));
	const float x = options.value("x_mm", 0.0f);
	const float y = options.value("y_mm", 0.0f);
	const std::string align = options.value("align", std::string("left"));
	const int layerId = options.value("layer_id", 1);
	const bool append = options.value("append", true);

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	if (text.empty() || fontPath.empty()) {
		if (out) {
			out->before = before;
			out->after = before;
			out->log = "Text: requires text and font_path";
		}
		return;
	}

	ofTrueTypeFont font;
	if (!font.load(fontPath, fontSize)) {
		if (out) {
			out->before = before;
			out->after = before;
			out->log = "Text: failed to load font";
		}
		ofLogWarning("TextProcessor") << "Could not load font: " << fontPath;
		return;
	}

	std::vector<ofPolyline> generated;
	for (const auto& path : font.getStringAsPoints(text, false, false)) {
		auto pl = pathToPolyline(path);
		if (pl.size() >= 2) generated.push_back(std::move(pl));
	}

	const float alignDx = textAlignOffsetX(generated, align);
	for (auto& pl : generated) {
		for (auto& v : pl.getVertices()) {
			v.x += x + alignDx;
			v.y += y;
		}
	}

	const size_t pathCount = generated.size();
	if (!append) {
		doc.paths.clear();
		doc.meta.clear();
	}
	for (auto& pl : generated) {
		StrokeMeta meta;
		meta.layerId = layerId;
		doc.paths.push_back(std::move(pl));
		doc.meta.push_back(meta);
	}
	doc.syncMetaSize();
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Added " + std::to_string(pathCount) + " text paths";
	}
}

} // namespace plotproc
