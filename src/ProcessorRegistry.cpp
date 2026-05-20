#include "ProcessorRegistry.h"
#include "processors/LineMergeProcessor.h"
#include "processors/LineSortProcessor.h"
#include "processors/FilterProcessor.h"
#include "processors/SimplifyProcessor.h"
#include "processors/ReloopProcessor.h"
#include "processors/ReverseProcessor.h"
#include "processors/SquigglesProcessor.h"
#include "processors/SplitAllProcessor.h"
#include "processors/MultipassProcessor.h"
#include "processors/SnapProcessor.h"
#include "processors/TranslateProcessor.h"
#include "processors/CropProcessor.h"

namespace plotproc {

ProcessorRegistry::ProcessorRegistry() {
	registerProcessor(std::make_unique<LineMergeProcessor>());
	registerProcessor(std::make_unique<LineSortProcessor>());
	registerProcessor(std::make_unique<FilterProcessor>());
	registerProcessor(std::make_unique<SimplifyProcessor>());
	registerProcessor(std::make_unique<ReloopProcessor>());
	registerProcessor(std::make_unique<ReverseProcessor>());
	registerProcessor(std::make_unique<SquigglesProcessor>());
	registerProcessor(std::make_unique<SplitAllProcessor>());
	registerProcessor(std::make_unique<MultipassProcessor>());
	registerProcessor(std::make_unique<SnapProcessor>());
	registerProcessor(std::make_unique<TranslateProcessor>());
	registerProcessor(std::make_unique<CropProcessor>());
}

ProcessorRegistry& ProcessorRegistry::instance() {
	static ProcessorRegistry reg;
	return reg;
}

void ProcessorRegistry::registerProcessor(std::unique_ptr<IPlotProcessor> processor) {
	if (!processor) return;
	m_processors.push_back(std::move(processor));
}

IPlotProcessor* ProcessorRegistry::get(const std::string& id) const {
	for (const auto& p : m_processors) {
		if (p && id == p->id()) return p.get();
	}
	return nullptr;
}

std::vector<std::string> ProcessorRegistry::ids() const {
	std::vector<std::string> out;
	for (const auto& p : m_processors) {
		if (p) out.push_back(p->id());
	}
	return out;
}

std::vector<const char*> ProcessorRegistry::displayNames() const {
	std::vector<const char*> out;
	for (const auto& p : m_processors) {
		if (p) out.push_back(p->displayName());
	}
	return out;
}

} // namespace plotproc
