# Makefile for rich-cpp (Items 14-28 Pure C++ Port Test Suite)

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -I.

TESTS = \
	tests/test_cross_language_parity \
	tests/test_golden_snapshots \
	tests/test_memory_safety \
	tests/test_thread_safety \
	tests/test_api_stability \
	tests/test_compiler_compat \
	tests/test_build_validation \
	tests/test_static_analysis \
	tests/test_code_quality \
	tests/test_performance_benchmarks \
	tests/test_fuzzing \
	tests/test_property_based \
	tests/test_doc_verification \
	tests/test_demo_validation \
	tests/test_judge_demo

.PHONY: all clean test_all judge_demo

all: rich_demo test_all

rich_demo: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

tests/test_%: tests/test_%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

test_all: $(TESTS)
	@echo "=== Built All Pure C++ Bonus Test Drivers (Items 14-28) ==="

judge_demo: tests/test_judge_demo
	./tests/test_judge_demo

clean:
	rm -f rich_demo tests/*.exe tests/test_*[!.]*
