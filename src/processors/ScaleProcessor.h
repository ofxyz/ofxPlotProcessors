#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class ScaleProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "scale"; }
	const char* displayName() const override { return "Scale"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
