#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class TranslateProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "translate"; }
	const char* displayName() const override { return "Translate"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
