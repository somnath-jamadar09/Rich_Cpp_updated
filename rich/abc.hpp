// Port of rich/abc.py
//
// Python's RichRenderable uses ABCMeta.__subclasshook__ to make isinstance()
// checks succeed for ANY type that merely happens to define __rich__ or
// __rich_console__, without explicit inheritance (structural typing).
// C++ has no runtime structural isinstance() equivalent; the nearest faithful
// port is a compile-time trait (see protocol.hpp's has_rich_method /
// has_rich_console_method) used as: `rich::is_rich_renderable_v<MyType>`
// in place of Python's `isinstance(my_object, RichRenderable)`.
#pragma once
#include "protocol.hpp"

namespace rich {

/// Compile-time equivalent of `isinstance(obj, RichRenderable)`.
/// Usage:
///   if constexpr (rich::is_rich_renderable_v<MyType>) { console.print(my_object); }
template <typename T>
constexpr bool is_rich_renderable_v =
    detail::has_rich_method<T>::value || detail::has_rich_console_method<T>::value;

} // namespace rich
