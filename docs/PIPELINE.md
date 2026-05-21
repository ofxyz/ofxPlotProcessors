# Pipeline

A **pipeline** is an ordered, enableable list of processors applied to one `StrokeDocument`.

## JSON preset format

```json
{
  "name": "plot_optimized",
  "steps": [
    {
      "id": "line_merge",
      "enabled": true,
      "options": { "tolerance_mm": 0.05, "allow_reverse": true }
    },
    {
      "id": "line_sort",
      "enabled": true,
      "options": { "allow_reverse": true, "two_opt": false }
    }
  ]
}
```

- `id` — processor `id()` (see [Processors](PROCESSORS.md))
- `enabled` — skip when `false`
- `options` — merged with processor `defaultOptions()` when empty

Load/save: `PlotPipeline::loadPreset(path)`, `savePreset(path)`.

Default file: `bin/data/plot_pipeline_default.json`.

## Recommended order

Typical pen-plot optimization:

1. **Structural** — `split_all` (dense meshes only) → `filter` → `simplify`
2. **Connect & order** — `line_merge` → `line_sort`
3. **Style** — `squiggles`, `reloop`, `multipass`
4. **Page** — `layout`, `trim`, `crop` / `circle_crop`
5. **Transforms** — `translate`, `scale`, `scale_to`, `rotate`, `skew` (as needed)

Put **`squiggles` after `line_sort`** so pen-up travel stays straight; wobble applies to drawn strokes only.

## Built-in defaults

`PlotPipeline::defaults()` enables:

- `line_merge` (tolerance 0.05 mm, allow reverse)
- `line_sort` (allow reverse, no 2-opt)
- `squiggles` — registered but **disabled**
- `filter` — **disabled**

## Metrics report

```cpp
auto report = pipe.runWithReport(doc);
// report.initial / report.final — PlotMetrics
// report.stepResults[] — per-step before/after + log string
```

Use `travelLengthMM` delta for “travel saved” in the UI.

## Recipes

**Standard export**

```json
["line_merge", "line_sort"]
```

**Dense mesh** (many short segments)

```json
["split_all", "line_merge", "line_sort"]
```

**Page fit (A4, 10 mm margin, centered)**

```json
["layout", "line_merge", "line_sort"]
```

`layout` options: `page_preset: "a4"`, `margin_mm: 10`, `fit_to_margins: true`.
