# Workflow: Porting & Verification Methodology

This document outlines the repeatable engineering workflow used to port Python's [Textualize/rich](https://github.com/Textualize/rich) to C++17, verify implementation correctness, and plan future module coverage.

---

## The 5-Step Porting Process

Every ported module follows this sequential workflow:

1. **Source Analysis**: Inspect the corresponding Python module in `rich/<name>.py` to understand its logic, invariants, and dependencies.
2. **Scope Determination**: Identify whether the complete module or a core subset is in scope (e.g. `color.py` core parsing was ported while color degradation was deferred).
3. **Idiomatic Translation**: Map Python idioms (duck-typing, keyword arguments, generators) to C++17 constructs (SFINAE traits, builder setters, vectors). Document architectural choices in [DECISIONS.md](DECISIONS.md).
4. **Static & Compilation Validation**: Compile headers against standard C++17 compilers (`g++ -std=c++17 -I. main.cpp`).
5. **Byte-for-Byte Output Verification**: Compare C++ generated ANSI sequences and text layout against live Python Rich output for identical test inputs.

---

## Verification Log

| Module | Verification Test | Result |
|---|---|---|
| [cells.hpp](rich/cells.hpp) | `cell_len()`, `chop_cells()`, and `set_cell_size()` executed on ASCII, CJK (`你好`), and emoji (`😀`) inputs across C++ and Python. | Identical cell width and crop boundaries. |
| [style.hpp](rich/style.hpp) | `Style::parse("bold red on blue").render("hi")` evaluated in C++ and Python Rich. | Identical ANSI escape sequence (`\x1b[1;31;44mhi\x1b[0m`). |
| [console.hpp](rich/console.hpp) | Nested markup `"[bold]a [red]b[/red] c[/bold]"` printed via `Console::print_markup()`. | Byte-identical ANSI output formatting. |
| [rule.hpp](rich/rule.hpp) | `Rule::print(40, "Title")` executed at width 40. | Identical rule dash distribution (16 left, 17 right). |
| [panel.hpp](rich/panel.hpp) | `Panel::print("Hi there", 20)` executed at width 20. | Identical rounded box-drawing border codepoints and padding. |
| [table.hpp](rich/table.hpp) | Multi-column, multi-row table rendered and diffed between C++ and Python output files. | Zero difference (`diff` clean). |

---

## Data Table Extractions

Data tables were extracted mechanically using Python's `ast` module rather than manually transcribed:

- **Unicode 17.0.0 Width Data** ([rich/_unicode_data_17_0_0.hpp](rich/_unicode_data_17_0_0.hpp)): 464 cell width ranges and 213 narrow-to-wide mappings extracted from `unicode17-0-0.py`.
- **ANSI Color Names** ([rich/_color_names.hpp](rich/_color_names.hpp)): 235 named colors extracted from `color.py`'s `ANSI_COLOR_NAMES` dictionary.

---

## Incremental Roadmap

Coverage expands in layered dependency order matching [ARCHITECTURE.md](ARCHITECTURE.md):

1. **Layer 1 Remainder**: `box.py`, `_palettes.py`, `default_styles.py`, `palette.py`, `theme.py`.
2. **Layer 2 Remainder**: `segment.py`, `text.py` (unlocking width-aware wrapping and justification).
3. **Layer 3 Expansion**: Full render protocol in `console.py`, `live.py`.
4. **Layer 4 Expansion**: `align.py`, `columns.py`, `padding.py`, `tree.py`, `layout.py`.
5. **High-Level Components**: `markdown.py`, `syntax.py`, `progress.py`, `json.py`, `traceback.py`.
