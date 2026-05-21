# Architecture

## Role in the stack

```
ofxPlotFinders   →  raster / image  →  strokes (scratch polylines → ofPath in plotter)
ofxPlotter       →  authoring, layers, export
ofxPlotProcessors →  optimize polylines before G-code (this addon)
```

**Canonical geometry in the app** is `ofPath` (one stroke per path). Processors work on **`ofPolyline`** at export/processing boundaries only.

## Core types

### `StrokeDocument`

- `paths` — one `ofPolyline` per pen-down stroke
- `meta` — parallel `StrokeMeta` (closed, `allowReverse`, `locked`, `layerId`, color, …)
- `bounds` — cached AABB; call `rebuildBounds()` after edits

### `IPlotProcessor`

Each processor implements:

- `id()` — stable key (e.g. `"line_merge"`)
- `displayName()` — UI label
- `process(doc, options, result?)` — in-place mutation
- `defaultOptions()` — `ofJson` defaults

### `ProcessorRegistry`

Singleton registry of built-in processors. Lookup by `id()` for pipeline steps and UI.

### `PlotPipeline`

Ordered list of `{ processorId, enabled, options }`. Methods:

- `run(doc)` / `runFrom(doc, stepIndex)`
- `runWithReport(doc)` — per-step `PlotMetrics` before/after
- `loadPreset(path)` / `savePreset(path)`
- `PlotPipeline::defaults()` — merge + sort on, squiggles/filter off

### `PlotMetrics`

`pathCount`, `vertexCount`, `drawLengthMM`, `travelLengthMM` — used for “travel saved” UI.

### `LineIndex`

Uniform-grid spatial index for endpoint merge/sort.

## Layer handling

Most geometry processors respect optional JSON filter:

```json
"layer_ids": [1, 2]
```

If omitted or empty, all layers are affected. Merge/sort/shuffle run **per layer** in first-seen layer order.

## Adding a processor

1. Subclass `IPlotProcessor` under `src/processors/`
2. Register in `ProcessorRegistry.cpp`
3. Document options in [Processors](PROCESSORS.md)
