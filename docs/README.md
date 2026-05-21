# ofxPlotProcessors docs

Native C++ stroke pipeline for pen plotters. Use these guides from **ofxPlotter** or any addon that depends on this library.

| Doc | Contents |
|-----|----------|
| [Architecture](ARCHITECTURE.md) | Types, data flow, extension points |
| [Pipeline](PIPELINE.md) | JSON presets, default recipes, metrics |
| [Processors](PROCESSORS.md) | All processors, options, typical order |

## Quick start

```cpp
#include "ofxPlotProcessors.h"

plotproc::StrokeDocument doc;  // fill from layer / finders
plotproc::PlotPipeline pipe = plotproc::PlotPipeline::loadPreset("bin/data/plot_pipeline_default.json");
pipe.run(doc);
```

Register custom steps via `ProcessorRegistry::instance().registerProcessor(...)`.

## Credits

Pipeline design is indebted to [vpype](https://github.com/abey79/vpype) by [Antoine Beyeler](https://github.com/abey79) (MIT). See the root [README](../README.md#credits).
