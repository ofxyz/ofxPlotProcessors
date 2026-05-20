#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class SplitAllProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "split_all"; }
	const char* displayName() const override { return "Split All"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
