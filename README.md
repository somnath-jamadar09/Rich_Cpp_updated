# rich-cpp — Port Mortem 2026 Submission

`rich-cpp` is a high-performance header-only C++17 port of [Textualize/rich](https://github.com/Textualize/rich), migrating Python's terminal formatting, ANSI color styling, BBCode-style markup parsing, and structured renderables (Rules, Panels, Tables) to C++. This project is submitted for **Port Mortem 2026** (Hackathon Raptors' Code Resurrection hackathon) under **Track H (Open Pair: Python → C++)**.

## Migration Rationale

Python's `rich` library is widely used for rich terminal formatting, but runtime duck-typing, dynamic memory allocation, and Python interpreter overhead limit its deployment in native systems programming, embedded environments, or performance-critical C++ applications. Porting `rich` to header-only C++17 achieves:
- **Zero Runtime Overhead & Dependencies**: Eliminates Python runtime dependencies, dynamic library linkages, and external third-party C++ libraries by building strictly on the C++17 Standard Library (`<string>`, `<vector>`, `<optional>`, `<iostream>`, `<unordered_map>`).
- **Compile-Time Renderable Trait Checking**: Replaces Python's dynamic runtime duck-typing (`__rich__` / `__rich_console__`) with zero-overhead C++ SFINAE traits (`rich::is_rich_renderable_v<T>`).
- **Significant Performance Gains**: Offers over 10x faster startup times, 7x lower memory footprint, and sub-millisecond table rendering latency compared to the original Python implementation.

## Build Instructions

Build a runnable artifact from a clean checkout using **one documented command**:

### Native C++ Compiler (GCC 7+, Clang 5+, MSVC 2017+):
```bash
g++ -std=c++17 -Wall -Wextra -Isrc src/main.cpp -o rich_demo && ./rich_demo
```

### Single-Command Container Build (Docker):
```bash
docker build -t port . && docker run --rm port
```

---

## Run and Test Instructions

### Running the C++ Port Demonstration
After compilation, execute the demonstration application:
```bash
./rich_demo
```

### Running Original Python Tests (`tests/original/`)
The original Python test suite from `Textualize/rich` has been preserved untouched in `tests/original/` (kickoff SHA256 hash: `226f6e340edd417c39cb85e657faaea828ca2ecba1b2260d08f68c9f1ad77a4f`).
To execute the baseline tests:
```bash
pytest tests/original/
```

### Running Port-Specific Tests (`tests/port/`)
Port-specific C++ test drivers verifying C++17 API stability, cross-language parity, memory safety, thread safety, golden snapshot rendering, and compiler compatibility:

**Using CMake:**
```bash
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
```

**Using Makefile / GCC:**
```bash
make test_all
```

**On Windows PowerShell / CMD:**
```cmd
run_all.bat
```

---

## License & Attribution

This project carries forward the original license and attribution of `Textualize/rich`.

```text
The MIT License (MIT)

Copyright (c) 2020 Will McGugan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY FROM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
LIABILITY FROM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
```

---

## Project Documentation & Artifact Links

- [DECISIONS.md](DECISIONS.md) — Architectural design decisions, divergences, and rationale
- [bench/](bench/) — Performance benchmark methodology ([bench/methodology.md](bench/methodology.md)) and verified results ([bench/results.json](bench/results.json))
- [fuzz/](fuzz/) — Differential fuzzing harness ([fuzz/harness.py](fuzz/harness.py)) and 60s+ verification log ([fuzz/log.txt](fuzz/log.txt))
- [.port-mortem.toml](.port-mortem.toml) — Hackathon submission tracking metadata
