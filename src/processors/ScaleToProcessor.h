#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class ScaleToProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "scale_to"; }
	const char* displayName() const override { return "Scale To"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
