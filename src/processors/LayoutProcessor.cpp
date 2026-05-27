#include "LayoutProcessor.h"
#include "ProcessorUtils.h"
#include "../PlotMetrics.h"
#include <algorithm>

namespace plotproc {

namespace {

float alignOffset(float contentMin, float contentMax, float pageMin, float pageMax, const std::string& align) {
	const float cw = contentMax - contentMin;
	const float pw = pageMax - pageMin;
	if (align == "left" || align == "top") return pageMin - contentMin;
	if (align == "right" || align == "bottom") return pageMax - contentMax;
	return pageMin + (pw - cw) * 0.5f - contentMin;
}

} // namespace

ofJson LayoutProcessor::defaultOptions() const {
	return {
		{"page_preset", "a4"},
		{"page_width_mm", 210.0},
		{"page_height_mm", 297.0},
		{"landscape", false},
		{"margin_mm", 0.0},
		{"fit_to_margins", true},
		{"align", "center"},
		{"valign", "center"},
		{"tight", false}
	};
}

void LayoutProcessor::process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) {
	float pageW = options.value("page_width_mm", 210.0f);
	float pageH = options.value("page_height_mm", 297.0f);
	if (options.contains("page_preset")) {
		const std::string preset = options["page_preset"].get<std::string>();
		if (preset != "tight") {
			pagePresetSizeMm(preset, pageW, pageH);
		}
	}
	const bool landscape = options.value("landscape", false);
	if (landscape) std::swap(pageW, pageH);

	const float margin = std::max(0.f, options.value("margin_mm", 0.0f));
	const bool fitMargins = options.value("fit_to_margins", true);
	const bool tight = options.value("tight", false);
	const std::string align = options.value("align", std::string("center"));
	const std::string valign = options.value("valign", std::string("center"));

	PlotMetrics before;
	if (out) before = PlotMetricsUtil::compute(doc);

	doc.rebuildBounds();
	if (doc.paths.empty()) {
		if (out) {
			out->before = before;
			out->after = before;
			out->log = "Layout: empty document";
		}
		return;
	}

	const ofRectangle content = doc.bounds;
	if (content.width < 1e-6f && content.height < 1e-6f) return;

	if (tight) {
		pageW = content.width + 2.f * margin;
		pageH = content.height + 2.f * margin;
	}

	const float innerW = std::max(1e-4f, pageW - 2.f * margin);
	const float innerH = std::max(1e-4f, pageH - 2.f * margin);

	float sx = 1.f;
	float sy = 1.f;
	if (fitMargins && !tight) {
		sx = innerW / content.width;
		sy = innerH / content.height;
		const float s = std::min(sx, sy);
		sx = sy = s;
	}

	const glm::vec2 origin = {content.x + content.width * 0.5f, content.y + content.height * 0.5f};
	for (auto& p : doc.paths) {
		scalePathAtOrigin(p, origin, sx, sy);
	}
	doc.rebuildBounds();

	const float tx = alignOffset(doc.bounds.x, doc.bounds.x + doc.bounds.width,
	                           margin, margin + innerW, align);
	const float ty = alignOffset(doc.bounds.y, doc.bounds.y + doc.bounds.height,
	                           margin, margin + innerH, valign);

	for (auto& p : doc.paths) {
		transformPathInPlace(p, [tx, ty](const glm::vec2& v) {
			return glm::vec2{v.x + tx, v.y + ty};
		});
	}
	doc.rebuildBounds();

	if (out) {
		out->before = before;
		out->after = PlotMetricsUtil::compute(doc);
		out->log = "Layout on " + ofToString(pageW, 1) + "x" + ofToString(pageH, 1) + " mm page";
	}
}

} // namespace plotproc
