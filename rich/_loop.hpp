// Port of rich/_loop.py
#pragma once
#include <vector>
#include <tuple>
#include <utility>

namespace rich {

/// Iterate and generate a tuple with a flag for first value.
template <typename Container>
std::vector<std::pair<bool, typename Container::value_type>> loop_first(const Container& values) {
    using T = typename Container::value_type;
    std::vector<std::pair<bool, T>> result;
    bool first = true;
    for (const auto& v : values) {
        result.emplace_back(first, v);
        first = false;
    }
    return result;
}

/// Iterate and generate a tuple with a flag for last value.
template <typename Container>
std::vector<std::pair<bool, typename Container::value_type>> loop_last(const Container& values) {
    using T = typename Container::value_type;
    std::vector<std::pair<bool, T>> result;
    size_t n = values.size();
    size_t i = 0;
    for (const auto& v : values) {
        result.emplace_back(i == n - 1, v);
        ++i;
    }
    return result;
}

/// Iterate and generate a tuple with a flag for first and last value.
template <typename Container>
std::vector<std::tuple<bool, bool, typename Container::value_type>> loop_first_last(
    const Container& values) {
    using T = typename Container::value_type;
    std::vector<std::tuple<bool, bool, T>> result;
    size_t n = values.size();
    size_t i = 0;
    for (const auto& v : values) {
        result.emplace_back(i == 0, i == n - 1, v);
        ++i;
    }
    return result;
}

} // namespace rich
