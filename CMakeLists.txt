cmake_minimum_required(VERSION 3.16)
project(rich_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rich_cpp INTERFACE)
target_include_directories(rich_cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rich)

add_executable(rich_demo main.cpp)
target_link_libraries(rich_demo PRIVATE rich_cpp)
target_compile_features(rich_demo PRIVATE cxx_std_17)

# Ported so far (header-only): errors, color_triplet, _loop, _pick, _stack,
# region, protocol, abc. See README.md for full status / roadmap.
