#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class RotateProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "rotate"; }
	const char* displayName() const override { return "Rotate"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
