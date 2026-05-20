#pragma once

#include "PlotProcessorTypes.h"
#include "StrokeDocument.h"
#include "ofJson.h"

namespace plotproc {

class IPlotProcessor {
public:
	virtual ~IPlotProcessor() = default;
	virtual const char* id() const = 0;
	virtual const char* displayName() const = 0;
	virtual ProcessorScope scope() const { return ProcessorScope::PerDocument; }
	virtual void process(StrokeDocument& doc,
	                     const ofJson& options,
	                     ProcessorResult* out = nullptr) = 0;
	virtual ofJson defaultOptions() const = 0;
};

} // namespace plotproc
