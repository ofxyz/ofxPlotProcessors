#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class SplitDistProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "split_dist"; }
	const char* displayName() const override { return "Split Dist"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
