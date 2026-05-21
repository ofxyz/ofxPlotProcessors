#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class CircleCropProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "circle_crop"; }
	const char* displayName() const override { return "Circle Crop"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
