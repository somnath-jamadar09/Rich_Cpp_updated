# Architecture

## Overview

`rich-cpp` is a **header-only C++17 library**. Every `.hpp` in `rich/`
corresponds 1:1 to a `.py` file in the original Rich package, mirroring
the Python module boundaries exactly so the two codebases stay easy to
cross-reference. There is no `.cpp`/compiled-library split (yet) — include
what you need, `#include "rich/console.hpp"` etc., and the compiler does
the rest. `main.cpp` is a standalone demo/smoke-test consumer of the
library, not part of the library itself.

## Layered dependency structure

Rich (and this port) is built in strict layers — each layer only depends
on layers below it, never sideways or up. This mirrors the real Python
package's import graph.

```
┌─────────────────────────────────────────────────────┐
│ Layer 4 — Renderables (visible output)               │
│   rule.hpp   panel.hpp   table.hpp                   │
│   (each depends on: cells.hpp for width, style.hpp    │
│    for color/attributes, <iostream> to write output) │
├─────────────────────────────────────────────────────┤
│ Layer 3 — Console (the print surface)                │
│   console.hpp                                        │
│   (depends on: style.hpp for ANSI rendering)          │
├─────────────────────────────────────────────────────┤
│ Layer 2 — Styling core                                │
│   color.hpp  (+ _color_names.hpp — generated data)   │
│   style.hpp  (depends on: color.hpp, errors.hpp)     │
├─────────────────────────────────────────────────────┤
│ Layer 1 — Text measurement                            │
│   cells.hpp  (depends on: _unicode_data.hpp)          │
│   _unicode_data.hpp (+ _unicode_data_17_0_0.hpp —    │
│    generated data)                                    │
├─────────────────────────────────────────────────────┤
│ Layer 0 — Foundations (no internal dependencies)      │
│   errors.hpp  color_triplet.hpp  region.hpp           │
│   _loop.hpp  _pick.hpp  _stack.hpp                    │
│   protocol.hpp  abc.hpp                                │
└─────────────────────────────────────────────────────┘
```

Rule: nothing in Layer *N* includes anything from Layer *N+1* or above.
This is enforced by convention (same as Python's import graph — Rich
avoids circular imports the same way) rather than by a build-system
constraint, since C++ headers don't have Python's import-cycle detection.

## Data flow: how a `console.print_markup(...)` call becomes terminal output

```
"[bold red]hi[/bold red]"
        │
        ▼
┌───────────────────┐
│  console.hpp       │  Scans for [tag]...[/tag] pairs, maintains a
│  print_markup()    │  style stack (nesting support)
└─────────┬──────────┘
          │  for each tag: Style::parse(tag_text)
          ▼
┌───────────────────┐
│  style.hpp          │  Parses "bold red" → sets bold=true,
│  Style::parse()     │  color=Color::parse("red")
└─────────┬──────────┘
          │  color name lookup
          ▼
┌───────────────────┐
│  color.hpp           │  "red" → ANSI_COLOR_NAMES lookup → Color{
│  Color::parse()      │    type=STANDARD, number=1 }
└─────────┬────────────┘
          │  style.render(text) combines attribute + color SGR codes
          ▼
┌───────────────────┐
│  style.hpp            │  ansi_codes() → "1;31"  (bold=1, red fg=31)
│  Style::render()      │  render() wraps: "\x1b[1;31m" + text + "\x1b[0m"
└─────────┬──────────────┘
          ▼
     std::cout  →  terminal
```

`table.hpp`/`panel.hpp`/`rule.hpp` follow a parallel, simpler path: they
call `cells::cell_len()` (Layer 1) directly to measure column/content
widths in *terminal cells* (not bytes, not codepoints — see below), then
write box-drawing Unicode characters + content directly to `std::cout`,
optionally wrapping styled portions through `style.hpp`'s `render()`.

## Why cell-width measurement is its own layer

Terminal output isn't measured in bytes or even Unicode codepoints — it's
measured in **cells** (the fixed-width character slots a terminal
allocates). A CJK character occupies 2 cells; most emoji occupy 2 cells;
combining characters (like skin-tone modifiers) occupy 0. Every renderable
in Layer 4 (`table.hpp` computing column widths, `panel.hpp` computing
padding, `rule.hpp` computing dash counts) needs this measurement to be
*exactly* right, or output misaligns. That's why `cells.hpp` sits as its
own foundational layer beneath styling, not folded into `console.hpp` —
matching the real Rich package's own module boundary (`cells.py` has zero
dependency on `style.py`/`console.py` either).

## Generated vs. hand-written files

Two files are **not hand-typed** — they're mechanically generated from the
real Python source data via a one-time extraction script (see
WORKFLOW.md for the exact process):

- `_unicode_data_17_0_0.hpp` — 464 Unicode width ranges + 213
  narrow-to-wide character mappings, extracted from
  `rich/_unicode_data/unicode17-0-0.py` via `ast.literal_eval`.
- `_color_names.hpp` — 235 named-color → ANSI-number mappings, extracted
  from `rich/color.py`'s `ANSI_COLOR_NAMES` dict the same way.

This separation matters architecturally: it means the *logic* files
(`cells.hpp`, `color.hpp`) are reviewable/auditable by a human, while the
*data* files are large, generated, and meant to be trusted the same way
you'd trust a generated protobuf file — regenerable from source, not
edited by hand.

## String representation choice

Two string types are used deliberately, not interchangeably:

- `std::string` (UTF-8 bytes) — used at API boundaries: function
  signatures, style definitions, markup text. Matches what a C++ caller
  naturally has on hand.
- `std::u32string` (UTF-32, one `char32_t` per codepoint) — used
  *internally* inside `cells.hpp` and anywhere that needs Python-string-
  like per-codepoint indexing (grapheme splitting, cropping). Converted
  from/to UTF-8 only at the function boundary.

See DECISIONS.md §6 for the reasoning.

## Extension points (where the next layer plugs in)

- **`segment.hpp`/`text.hpp`** (not yet built) would sit between Layer 1
  and Layer 3 — a `Segment` is a `(text, style)` pair, and `Text` is a
  sequence of segments with word-wrap/justify logic built on `cells.hpp`'s
  measurement. `console.hpp`'s `print()` would then route through
  `Text::wrap()` instead of writing raw strings, gaining width-aware
  line-splitting.
- **`box.hpp`** (not yet built) would generalize the hard-coded box-drawing
  characters currently inlined in `table.hpp`/`panel.hpp` into swappable
  `Box` definitions (ROUNDED, HEAVY, SQUARE, ...), matching Python's
  `box.py`.
