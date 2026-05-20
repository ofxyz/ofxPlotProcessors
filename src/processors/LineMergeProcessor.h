#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class LineMergeProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "line_merge"; }
	const char* displayName() const override { return "Merge strokes"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
