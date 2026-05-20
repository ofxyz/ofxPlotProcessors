#include "PlotPipeline.h"
#include "ProcessorRegistry.h"
#include "PlotMetrics.h"
#include "ofUtils.h"

namespace plotproc {

void PlotPipeline::run(StrokeDocument& doc) const {
	runFrom(doc, 0);
}

void PlotPipeline::runFrom(StrokeDocument& doc, size_t startStep) const {
	for (size_t i = startStep; i < steps.size(); ++i) {
		const auto& step = steps[i];
		if (!step.enabled) continue;
		IPlotProcessor* proc = ProcessorRegistry::instance().get(step.processorId);
		if (!proc) {
			ofLogWarning("PlotPipeline") << "Unknown processor: " << step.processorId;
			continue;
		}
		ofJson opts = step.options;
		if (opts.is_null() || (opts.is_object() && opts.empty())) {
			opts = proc->defaultOptions();
		}
		proc->process(doc, opts, nullptr);
	}
}

PipelineRunReport PlotPipeline::runWithReport(StrokeDocument& doc) const {
	PipelineRunReport report;
	report.initial = PlotMetricsUtil::compute(doc);
	for (const auto& step : steps) {
		if (!step.enabled) continue;
		IPlotProcessor* proc = ProcessorRegistry::instance().get(step.processorId);
		if (!proc) continue;
		ofJson opts = step.options;
		if (opts.is_null() || (opts.is_object() && opts.empty())) {
			opts = proc->defaultOptions();
		}
		ProcessorResult stepResult;
		proc->process(doc, opts, &stepResult);
		report.stepResults.push_back(stepResult);
	}
	report.final = PlotMetricsUtil::compute(doc);
	return report;
}

PlotPipeline PlotPipeline::defaults() {
	PlotPipeline pipe;
	auto& reg = ProcessorRegistry::instance();
	auto addStep = [&](const char* id, bool enabled) {
		PipelineStep step;
		step.processorId = id;
		step.enabled = enabled;
		if (auto* p = reg.get(id)) step.options = p->defaultOptions();
		pipe.steps.push_back(step);
	};
	addStep("line_merge", true);
	addStep("line_sort", true);
	addStep("squiggles", false);
	addStep("filter", false);
	return pipe;
}

PlotPipeline PlotPipeline::loadPreset(const std::string& path) {
	PlotPipeline pipe = defaults();
	ofJson json = ofLoadJson(path);
	if (json.is_null()) return pipe;
	if (json.contains("steps") && json["steps"].is_array()) {
		pipe.steps.clear();
		for (const auto& s : json["steps"]) {
			PipelineStep step;
			step.processorId = s.value("id", "");
			step.enabled = s.value("enabled", true);
			if (s.contains("options")) step.options = s["options"];
			if (!step.processorId.empty()) pipe.steps.push_back(step);
		}
	}
	return pipe;
}

bool PlotPipeline::savePreset(const std::string& path) const {
	ofJson json;
	json["name"] = "custom";
	ofJson stepsJson = ofJson::array();
	for (const auto& s : steps) {
		ofJson entry;
		entry["id"] = s.processorId;
		entry["enabled"] = s.enabled;
		entry["options"] = s.options;
		stepsJson.push_back(entry);
	}
	json["steps"] = stepsJson;
	return ofSaveJson(path, json);
}

} // namespace plotproc
