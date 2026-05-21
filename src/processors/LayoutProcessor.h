#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class LayoutProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "layout"; }
	const char* displayName() const override { return "Layout"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
