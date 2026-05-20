#pragma once

#include "PlotProcessorTypes.h"
#include "StrokeDocument.h"
#include "ofJson.h"
#include <string>
#include <vector>

namespace plotproc {

struct PipelineStep {
	std::string processorId;
	bool enabled = true;
	ofJson options;
};

struct PipelineRunReport {
	PlotMetrics initial;
	PlotMetrics final;
	std::vector<ProcessorResult> stepResults;
};

class PlotPipeline {
public:
	std::vector<PipelineStep> steps;

	void run(StrokeDocument& doc) const;
	void runFrom(StrokeDocument& doc, size_t startStep) const;
	PipelineRunReport runWithReport(StrokeDocument& doc) const;

	static PlotPipeline defaults();
	static PlotPipeline loadPreset(const std::string& path);
	bool savePreset(const std::string& path) const;
};

} // namespace plotproc
