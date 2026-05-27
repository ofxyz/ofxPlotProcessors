# ofxPlotProcessors

Composable **stroke pipeline** for pen plotters: merge, sort, filter, transform, layout, and clip on `ofPath` strokes before G-code export.

Works with **[ofxPlotFinders](https://github.com/ofxyz/ofxPlotFinders)** (image → strokes) and **ofxPlotter** (ECS, UI, export).

## Quick start

```cpp
#include "ofxPlotProcessors.h"

plotproc::StrokeDocument doc = /* from layer or finders */;
plotproc::PlotPipeline pipe = plotproc::PlotPipeline::loadPreset("bin/data/plot_pipeline_default.json");
pipe.run(doc);
```

## Documentation

| Doc | Contents |
|-----|----------|
| [docs/README.md](docs/README.md) | Index |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | `StrokeDocument`, registry, layers |
| [docs/PIPELINE.md](docs/PIPELINE.md) | JSON presets, recipes, metrics |
| [docs/PROCESSORS.md](docs/PROCESSORS.md) | All processors and options |

## Default pipeline

`bin/data/plot_pipeline_default.json` — `line_merge` → `line_sort` (squiggles/filter optional).

## Credits

Many processors in this addon follow ideas from [**vpype**](https://github.com/abey79/vpype) by [**Antoine Beyeler**](https://github.com/abey79) (MIT)—a brilliant toolchain for pen-plot workflows. Thank you, Antoine.

ofxPlotProcessors is a native openFrameworks library for in-app pipelines; it is not a port of vpype.