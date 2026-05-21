#pragma once
#include "../IPlotProcessor.h"

namespace plotproc {

/// Stroke text via ``ofTrueTypeFont`` (TTF). vpype uses Hershey fonts — different source, similar role.
class TextProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "text"; }
	const char* displayName() const override { return "Text"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out = nullptr) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
