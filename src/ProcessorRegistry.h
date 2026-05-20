#pragma once

#include "IPlotProcessor.h"
#include <memory>
#include <vector>

namespace plotproc {

class ProcessorRegistry {
public:
	static ProcessorRegistry& instance();

	void registerProcessor(std::unique_ptr<IPlotProcessor> processor);
	IPlotProcessor* get(const std::string& id) const;
	std::vector<std::string> ids() const;
	std::vector<const char*> displayNames() const;

private:
	ProcessorRegistry();
	std::vector<std::unique_ptr<IPlotProcessor>> m_processors;
};

} // namespace plotproc
