#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class SnapProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "snap"; }
	const char* displayName() const override { return "Snap Grid"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
