#pragma once

#include "../IPlotProcessor.h"

namespace plotproc {

class LineSortProcessor : public IPlotProcessor {
public:
	const char* id() const override { return "line_sort"; }
	const char* displayName() const override { return "Order strokes"; }
	void process(StrokeDocument& doc, const ofJson& options, ProcessorResult* out) override;
	ofJson defaultOptions() const override;
};

} // namespace plotproc
