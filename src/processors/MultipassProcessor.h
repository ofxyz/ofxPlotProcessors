#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

class MultipassProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "multipass"; }
	const char* displayName() const override { return "Multipass"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
