# Processors

All processors mutate a `StrokeDocument` in place. Each stroke is an **`ofPath`**. Options are JSON; units are **millimetres** unless noted.

**Curve preservation:** scale, rotate, translate, skew, and merge (forward joins) keep bezier commands. Simplify, snap, clip, squiggles, and split tessellate to polylines internally.

Common option on transforms and crops:

| Option | Type | Description |
|--------|------|-------------|
| `layer_ids` | `[int]` | Only affect these layers (default: all) |
| `origin_x_mm`, `origin_y_mm` | float | Transform origin (default: content bbox centre) |

---

## Optimize

### `line_merge`

Joins strokes whose endpoints are within tolerance (per layer).

| Option | Default | Description |
|--------|---------|-------------|
| `tolerance_mm` | `0.05` | Max gap to merge |
| `allow_reverse` | `true` | May flip stroke direction |

### `line_sort`

Greedy reorder to reduce pen-up travel (per layer).

| Option | Default | Description |
|--------|---------|-------------|
| `allow_reverse` | `true` | May flip strokes |
| `two_opt` | `false` | Optional 2-opt pass |
| `passes` | `1` | Sort passes |

### `split_all`

Splits every stroke into 2-point segments. Use before merge on dense meshes.

### `filter`

Removes strokes by length or open/closed.

| Option | Default |
|--------|---------|
| `min_length_mm` | `0` |
| `max_length_mm` | large |
| `closed_only` / `open_only` | `false` |
| `closed_tolerance_mm` | `0.05` |

### `simplify`

Reduces vertices by tessellating each path, running `ofPolyline::simplify`, then rebuilding as line segments.

| Option | Default |
|--------|---------|
| `tolerance_mm` | `0.05` |

### `line_shuffle`

Randomizes stroke order within each layer (`seed`, `-1` = random).

### `split_dist`

Splits overflow strokes into new layer IDs when cumulative draw length exceeds limit.

| Option | Default |
|--------|---------|
| `max_draw_mm` | `1000` |

---

## Style

### `squiggles`

Resamples strokes and adds 2D Perlin displacement (hand-drawn wobble).

| Option | Default |
|--------|---------|
| `amplitude_mm` | `0.5` |
| `period_mm` | `3.0` |
| `quantization_mm` | `0.05` |
| `seed` | `0` |

### `reloop`

Moves the seam on closed paths (randomized).

| Option | Default |
|--------|---------|
| `tolerance_mm` | `0.05` |
| `seed` | `-1` |

### `multipass`

Duplicates strokes for multi-pass plotting.

| Option | Default |
|--------|---------|
| `passes` | `2` |

### `snap`

Snaps vertices to a grid.

| Option | Default |
|--------|---------|
| `pitch_mm` | `1.0` |

### `reverse`

| Option | Default |
|--------|---------|
| `reverse_order` | `false` — reverse stroke list |
| `flip_lines` | `false` — reverse each path’s draw direction |

---

## Transform

### `translate`

| Option | Default |
|--------|---------|
| `offset_x_mm` | `0` |
| `offset_y_mm` | `0` |

### `scale`

| Option | Default |
|--------|---------|
| `scale` or `scale_x` / `scale_y` | `1` |

### `scale_to`

Fits content bbox to target size (uniform unless `fit_dimensions`).

| Option | Default |
|--------|---------|
| `width_mm` | `100` |
| `height_mm` | `100` |
| `fit_dimensions` | `false` — independent X/Y scale |

### `rotate`

| Option | Default |
|--------|---------|
| `angle_deg` | `0` — clockwise, y-down coords |

### `skew`

| Option | Default |
|--------|---------|
| `skew_x_deg` | `0` |
| `skew_y_deg` | `0` |

### `page_rotate`

Rotates all geometry **90°** (counter-clockwise default; `clockwise: true` for CW).

---

## Page & clip

### `layout`

Scales and positions content on a page.

| Option | Default | Notes |
|--------|---------|-------|
| `page_preset` | — | `a4`, `a3`, `letter`, `tight`, … |
| `page_width_mm` / `page_height_mm` | `210` / `297` | Used if no preset |
| `landscape` | `false` | Swaps page W/H |
| `margin_mm` | `0` | Inset |
| `fit_to_margins` | `true` | Uniform scale to fit |
| `align` / `valign` | `center` | `left` `center` `right` / `top` `center` `bottom` |
| `tight` | `false` | Page size = content + margin only |

### `trim`

Crops to content bbox inset by margin.

| Option | Default |
|--------|---------|
| `margin_x_mm` | `1` |
| `margin_y_mm` | `1` |

### `crop`

Rectangular clip.

| Option | Default |
|--------|---------|
| `x_mm`, `y_mm` | `0` |
| `width_mm`, `height_mm` | `100` |

### `circle_crop`

Circular clip.

| Option | Default |
|--------|---------|
| `center_x_mm`, `center_y_mm` | `0` |
| `radius_mm` | `50` |

---

## Generate

### `text`

Appends stroke outlines from a TTF (not enabled in default pipeline).

| Option | Default |
|--------|---------|
| `text` | `""` |
| `font_path` | required |
| `font_size` | `18` |
| `x_mm`, `y_mm` | `0` |
| `align` | `left` — `left` `center` `right` |
| `layer_id` | `1` |
| `append` | `true` — `false` replaces document |
