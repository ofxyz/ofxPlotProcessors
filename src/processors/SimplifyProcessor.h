#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class SimplifyProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "simplify"; }
	const char* displayName() const override { return "Simplify"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
