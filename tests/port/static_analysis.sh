#!/usr/bin/env bash
# Item 21: Static Analysis Automation Script
# Executes clang-tidy and cppcheck static analyzers over rich-cpp header files.

set -e

echo "=== Running Cppcheck Static Analysis ==="
if command -v cppcheck &> /dev/null; then
    cppcheck --enable=all \
             --inline-suppr \
             --suppress=missingIncludeSystem \
             --suppress=unusedFunction \
             --std=c++17 \
             -I rich \
             rich/*.hpp main.cpp
else
    echo "cppcheck not found on PATH. Skipping cppcheck execution."
fi

echo "=== Running Clang-Tidy Static Analysis ==="
if command -v clang-tidy &> /dev/null; then
    clang-tidy rich/*.hpp main.cpp -- -std=c++17 -I.
else
    echo "clang-tidy not found on PATH. Skipping clang-tidy execution."
fi

echo "Static Analysis Audit Completed Successfully."
