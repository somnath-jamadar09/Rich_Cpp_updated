# rich-cpp

A C++17 port of [Textualize/rich](https://github.com/Textualize/rich) — built
for **[hackathon name] hackathon**. Folder layout mirrors the original Python
repo 1:1; each `rich/<name>.py` becomes `rich/<name>.hpp`.

See also: **[DECISIONS.md](DECISIONS.md)** (porting decisions),
**[ARCHITECTURE.md](ARCHITECTURE.md)** (system design),
**[WORKFLOW.md](WORKFLOW.md)** (how it was built, verification process).

## Tech stack

| Layer | Choice | Why |
|---|---|---|
| Language | C++17 | Needed `std::optional`, structured bindings, `if constexpr` for the duck-typing → SFINAE port (see DECISIONS.md) |
| Build system | CMake ≥ 3.16 | Cross-platform standard; project ships a working `CMakeLists.txt` |
| Compiler (tested) | GCC (w64devkit / MSYS2 on Windows, any modern g++ on Linux/Mac) | No compiler-specific extensions used — any C++17-conformant compiler works |
| Dependencies | **None external** | Standard library only (`<string>`, `<vector>`, `<optional>`, `<regex>`, `<unordered_map>`, `<codecvt>`) — no vcpkg/conan package needed |
| Data generation | Python 3 (`ast` module) | Used *offline*, at port-authoring time only, to mechanically extract real Python data tables (Unicode widths, ANSI color names) into C++ headers — not a runtime dependency |
| Verification | Python 3 + live `pip install rich` | Used *offline* to diff C++ output against real Rich output byte-for-byte — not a runtime dependency |

**Runtime dependency footprint: zero.** Anyone building this needs only a
C++17 compiler. Python is not required to run the compiled program — only
to reproduce the verification steps documented in WORKFLOW.md.

## Things to install to run this

1. **A C++17 compiler**
   - Windows: [w64devkit](https://github.com/skeeto/w64devkit/releases) (self-extracting `.7z.exe`, no PATH setup needed if you call it by full path) or MSYS2 (`pacman -S mingw-w64-x86_64-gcc`)
   - macOS: `xcode-select --install`
   - Linux: `sudo apt install build-essential`
2. **(Optional) CMake ≥ 3.16** — only needed if you want to use `CMakeLists.txt` instead of calling `g++` directly.

That's it — no package manager, no third-party libraries.

## Build & run

**Quick (no CMake, works everywhere):**
```bash
g++ -std=c++17 -I. main.cpp -o rich_demo
./rich_demo          # Windows: rich_demo.exe
```

**Windows PowerShell** (if compiler isn't on PATH):
```powershell
$env:Path = "<path-to-w64devkit>\bin;" + $env:Path
chcp 65001                                    # UTF-8 output (box-drawing chars, CJK)
g++ -std=c++17 -I. main.cpp -o rich_demo.exe
.\rich_demo.exe
```

**With CMake:**
```bash
cmake -S . -B build
cmake --build build
./build/rich_demo
```

## Status: partial — 16 / 77 modules ported (+ 2 data tables)

Real, faithful ports done so far (no stubs, cross-checked against actual
Python `rich` output where behavior is non-trivial):

| Python file | C++ file | Notes |
|---|---|---|
| `errors.py` | `errors.hpp` | Exception hierarchy → `std::runtime_error` hierarchy |
| `color_triplet.py` | `color_triplet.hpp` | `NamedTuple` → `struct` |
| `_loop.py` | `_loop.hpp` | Generators → `std::vector` of results (templated) |
| `_pick.py` | `_pick.hpp` | `Optional[bool]` → `std::optional<bool>` |
| `_stack.py` | `_stack.hpp` | `List[T]` subclass → `std::vector<T>` subclass |
| `region.py` | `region.hpp` | `NamedTuple` → `struct` |
| `protocol.py` | `protocol.hpp` | Runtime duck-typing (`hasattr`) → compile-time detection idiom (SFINAE traits) |
| `abc.py` | `abc.hpp` | `ABCMeta.__subclasshook__` → `constexpr` trait `is_rich_renderable_v<T>` |
| `_unicode_data/__init__.py` + `unicode17-0-0.py` | `_unicode_data.hpp` + `_unicode_data_17_0_0.hpp` | Loader ported; width table (464 ranges) + narrow-to-wide set (213 entries) mechanically extracted from real Python data via `ast.literal_eval`, not hand-typed |
| `cells.py` | `cells.hpp` | Cell-width measurement, grapheme splitting, text cropping/padding/chopping. **Verified** against live `rich.cells` (ASCII/CJK/emoji) |
| `color.py` (core) | `color.hpp` + `_color_names.hpp` | `Color::parse` (named/`#hex`/`color(N)`/`rgb(r,g,b)`), `get_ansi_codes()`. 235 named colors mechanically extracted. Not ported: `downgrade()`, `get_truecolor()` |
| `style.py` (core) | `style.hpp` | Full bit-per-attribute model, `Style::parse(...)`, combination, `render()`. **Verified** byte-identical to live Python |
| `console.py` (subset) | `console.hpp` | `print(text, style)`, `print_markup(...)` with real nested tags. **Verified** byte-identical to live Python. Not ported: ~2650 lines (wrapping, justify, Table/Live rendering) |
| `rule.py` (core) | `rule.hpp` | Centered rule with title. **Verified** byte-identical to live Python |
| `panel.py` (core) | `panel.hpp` | Default ROUNDED box. **Verified** byte-identical to live Python |
| `table.py` (core) | `table.hpp` | Default HEAVY_HEAD box, auto column width, centered title. **Verified**: `diff`'d byte-for-byte against live Python, zero differences |

Full verification methodology, why only 16/77, and the dependency-ordered
roadmap for the rest are in **[WORKFLOW.md](WORKFLOW.md)**.

## Folder structure

Mirrors the original repo layout exactly (`.faq/`, `.github/`, `assets/`,
`benchmarks/`, `docs/`, `examples/`, `imgs/`, `questions/`, `rich/`,
`tests/`, `tools/`, `.coveragerc`, `.gitignore`, `.pre-commit-config.yaml`,
`.readthedocs.yml`, `LICENSE`, `Makefile`). Folders other than `rich/` are
empty placeholders reserved for future tests/docs/tooling — not filled
with fabricated content.
