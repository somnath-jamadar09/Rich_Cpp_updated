# Benchmark Methodology — rich-cpp (Port Mortem 2026)

This document describes the benchmark setup, measurement metrics, and comparative evaluation methodology between the original Python `rich` library and the ported C++ `rich-cpp` library.

## Measured Metrics

1. **Startup Time (`startup_ms`)**:
   - Time required from program invocation / module import to readiness for console rendering.
   - Python: Measured using `time.perf_counter()` wrapping `import rich` and `Console()` instantiation.
   - C++: Measured from process launch to execution of `main()`.

2. **Memory Footprint (`rss_mb`)**:
   - Resident Set Size (RSS) memory consumption in megabytes.
   - Measured via `psutil.Process().memory_info().rss`.

3. **P99 Latency (`p99_latency_ms`)**:
   - 99th percentile latency across 1,000 table render operations containing multi-column ANSI styled text and borders.

4. **Throughput (`throughput_rps`)**:
   - Total rendered components per second (`render_operations / total_time_sec`).

## Environment & Conditions

- **Operating System**: Windows / Linux Docker container
- **Compiler**: GCC 12 (`-std=c++17 -O3 -Isrc`)
- **Python Version**: Python 3.11 / Python 3.13
- **Test Workload**: Formatted 2x2 data tables with titles, cell measurement, ANSI escape rendering, and rounded box-drawing borders.
- **Warm-up**: 100 un-timed iterations executed prior to latency recording.

## Reproducing Benchmarks

Run the automated benchmark runner:
```bash
python bench/run_benchmarks.py
```
Outputs are recorded directly into [bench/results.json](results.json).
