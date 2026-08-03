// Port of rich/protocol.py
//
// NOTE ON PORTING APPROACH:
// Python's rich_cast/is_renderable rely on dynamic duck-typing (hasattr checks
// for __rich__ / __rich_console__) resolved at runtime. C++ has no direct
// equivalent, so this is ported using the "detection idiom" (compile-time
// SFINAE traits) which is the standard C++ approach for the same intent:
// "does this type expose a rich_render()/rich_console() member?".
#pragma once
#include <type_traits>
#include <string>
#include <typeindex>
#include <unordered_set>

namespace rich {

// Forward declaration; RenderableType is defined fully in console.hpp
class RenderableType;

namespace detail {

template <typename T, typename = void>
struct has_rich_method : std::false_type {};

template <typename T>
struct has_rich_method<T, std::void_t<decltype(std::declval<T>().rich())>>
    : std::true_type {};

template <typename T, typename = void>
struct has_rich_console_method : std::false_type {};

template <typename T>
struct has_rich_console_method<
    T, std::void_t<decltype(std::declval<T>().rich_console())>> : std::true_type {};

} // namespace detail

/// Check if a type may be rendered by Rich (has rich() or rich_console()).
template <typename T>
constexpr bool is_renderable_type() {
    return std::is_same<std::decay_t<T>, std::string>::value ||
           detail::has_rich_method<T>::value ||
           detail::has_rich_console_method<T>::value;
}

/// Runtime check mirroring is_renderable() for a std::string special case.
inline bool is_renderable(const std::string&) { return true; }

/// Cast an object to a renderable by calling rich() repeatedly if present.
/// Guards against infinite loops the same way the Python version does,
/// by tracking visited runtime types.
template <typename T>
auto rich_cast(T renderable) {
    if constexpr (detail::has_rich_method<T>::value) {
        std::unordered_set<std::type_index> visited;
        auto current = renderable;
        while (true) {
            auto next = current.rich();
            std::type_index t = typeid(next);
            if (visited.count(t)) break;
            visited.insert(t);
            if constexpr (detail::has_rich_method<decltype(next)>::value) {
                current = next;
                continue;
            } else {
                return next;
            }
        }
        return current.rich();
    } else {
        return renderable;
    }
}

} // namespace rich
