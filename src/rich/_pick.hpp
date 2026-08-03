// Port of rich/_pick.py
#pragma once
#if __has_include(<optional>)
#include <optional>
#elif __has_include(<experimental/optional>)
#include <experimental/optional>
namespace std { using experimental::optional; using experimental::nullopt; }
#endif
#include <initializer_list>
#include <cassert>

namespace rich {

/// Pick the first non-nullopt bool or return the last value.
/// Args: values - any number of optional<bool> values.
/// Returns: first non-nullopt boolean (falls back to bool of last if all empty).
inline bool pick_bool(std::initializer_list<std::optional<bool>> values) {
    assert(values.size() > 0 && "1 or more values required");
    bool last = false;
    for (const auto& value : values) {
        if (value.has_value()) {
            return *value;
        }
        last = false; // mirrors Python's fallback to bool(None) == False
    }
    return last;
}

} // namespace rich
