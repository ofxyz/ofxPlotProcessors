#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class SkewProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "skew"; }
	const char* displayName() const override { return "Skew"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
