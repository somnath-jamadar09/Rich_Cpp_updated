# Makefile for rich-cpp (Items 14-28 Pure C++ Port Test Suite)

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Isrc

TESTS = \
	tests/port/test_cross_language_parity \
	tests/port/test_golden_snapshots \
	tests/port/test_memory_safety \
	tests/port/test_thread_safety \
	tests/port/test_api_stability \
	tests/port/test_compiler_compat \
	tests/port/test_build_validation \
	tests/port/test_static_analysis \
	tests/port/test_code_quality \
	tests/port/test_performance_benchmarks \
	tests/port/test_fuzzing \
	tests/port/test_property_based \
	tests/port/test_doc_verification \
	tests/port/test_demo_validation \
	tests/port/test_judge_demo

.PHONY: all clean test_all judge_demo

all: rich_demo test_all

rich_demo: src/main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

tests/port/test_%: tests/port/test_%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

test_all: $(TESTS)
	@echo "=== Built All Pure C++ Bonus Test Drivers (Items 14-28) ==="

judge_demo: tests/port/test_judge_demo
	./tests/port/test_judge_demo

clean:
	rm -f rich_demo tests/port/*.exe tests/port/test_*[!.]*
