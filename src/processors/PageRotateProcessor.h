#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

/// Rotates all geometries 90° (vpype ``pagerotate`` also toggles page metadata in CLI).
class PageRotateProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "page_rotate"; }
	const char* displayName() const override { return "Page Rotate"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
