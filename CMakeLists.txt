cmake_minimum_required(VERSION 3.16)
project(rich_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rich_cpp INTERFACE)
target_include_directories(rich_cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(rich_demo main.cpp)
target_link_libraries(rich_demo PRIVATE rich_cpp)
target_compile_features(rich_demo PRIVATE cxx_std_17)

# Items 14–28 Bonus Test Targets
enable_testing()

set(TEST_FILES
    test_cross_language_parity
    test_golden_snapshots
    test_memory_safety
    test_thread_safety
    test_api_stability
    test_compiler_compat
    test_build_validation
    test_static_analysis
    test_code_quality
    test_performance_benchmarks
    test_fuzzing
    test_property_based
    test_doc_verification
    test_demo_validation
    test_judge_demo
)

foreach(test_name ${TEST_FILES})
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE rich_cpp)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
