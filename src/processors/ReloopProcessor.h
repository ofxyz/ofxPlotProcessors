#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class ReloopProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "reloop"; }
	const char* displayName() const override { return "Reloop closed"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
