#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class SquigglesProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "squiggles"; }
	const char* displayName() const override { return "Squiggles"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
