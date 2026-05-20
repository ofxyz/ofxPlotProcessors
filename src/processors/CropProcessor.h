#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class CropProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "crop"; }
	const char* displayName() const override { return "Crop Rect"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
