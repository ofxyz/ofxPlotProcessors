#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class ReverseProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "reverse"; }
	const char* displayName() const override { return "Reverse all"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
