#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class LineShuffleProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "line_shuffle"; }
	const char* displayName() const override { return "Line Shuffle"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
