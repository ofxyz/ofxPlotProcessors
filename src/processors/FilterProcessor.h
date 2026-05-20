#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class FilterProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "filter"; }
	const char* displayName() const override { return "Filter strokes"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
