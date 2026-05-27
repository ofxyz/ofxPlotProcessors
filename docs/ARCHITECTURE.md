# Architecture

## Role in the stack

```
ofxPlotFinders   →  raster / image  →  strokes (ofPath in plotter layers)
ofxPlotter       →  authoring, layers, export (ImageToPath::toStrokeDocument)
ofxPlotProcessors →  optimize paths before G-code (this addon)
```

**Canonical geometry** in the pipeline is **`ofPath`** — one pen-down stroke per path, with bezier curves preserved where possible.

Affine processors (scale, rotate, translate, skew, layout) transform control points directly. Vertex-level processors (simplify, snap, clip, squiggles) tessellate internally when needed, then rebuild as `lineTo` paths.

## Core types

### `StrokeDocument`

- `paths` — one `ofPath` per pen-down stroke
- `meta` — parallel `StrokeMeta` (closed, `allowReverse`, `locked`, `layerId`, color, …)
- `bounds` — cached AABB from path commands; call `rebuildBounds()` after edits

Helper accessors: `pathStart(index)`, `pathEnd(index)`, `pathLengthMM(index)` (tessellated length for metrics).

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

`vertexCount` and draw length are derived from tessellated outlines (for consistent metrics across curved paths).

### `LineIndex`

Uniform-grid spatial index for path endpoint merge/sort (`pathStartPt` / `pathEndPt`).

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
