#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class TrimProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "trim"; }
	const char* displayName() const override { return "Trim"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
