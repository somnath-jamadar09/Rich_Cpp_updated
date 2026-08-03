# Porting Decisions: Python → C++

This document records technical design decisions made during the port of [Textualize/rich](https://github.com/Textualize/rich) from Python to C++17.

---

## 1. Duck-Typing → Compile-Time SFINAE Traits ([protocol.hpp](rich/protocol.hpp), [abc.hpp](rich/abc.hpp))

**Python Behavior:**
Python Rich uses `isinstance(obj, RichRenderable)` at runtime to inspect objects for `__rich__` or `__rich_console__` methods via `ABCMeta.__subclasshook__`.

**C++ Decision:**
C++ lacks runtime duck-typing without inheritance hierarchies. `rich-cpp` uses SFINAE detection traits (`has_rich_method<T>`, `has_rich_console_method<T>`) resolved at compile time via `std::void_t`. The trait `rich::is_rich_renderable_v<T>` evaluates whether a type is renderable at compile time using `if constexpr`.

---

## 2. `NamedTuple` → C++ `struct` ([color_triplet.hpp](rich/color_triplet.hpp), [region.hpp](rich/region.hpp))

**Python Behavior:**
`ColorTriplet` and `Region` are `NamedTuple` subclasses (immutable, unpackable containers with structural equality).

**C++ Decision:**
Ported as standard C++ `struct` types with public fields. `operator==` is implemented explicitly where structural comparison is required (e.g. `ColorTriplet`).

---

## 3. Exceptions Hierarchy ([errors.hpp](rich/errors.hpp))

**Python Behavior:**
Python Rich defines a hierarchy of custom `Exception` classes in `errors.py`.

**C++ Decision:**
Mirrored using custom exception classes deriving from `std::runtime_error` in [errors.hpp](rich/errors.hpp). The class inheritance hierarchy matches Python so `catch` blocks operate equivalently.

---

## 4. `Optional[T]` → `std::optional<T>` ([_pick.hpp](rich/_pick.hpp))

**Python Behavior:**
Uses `Optional[bool]` or `Optional[T]` to denote nullable return values and default parameters.

**C++ Decision:**
Mapped directly to C++17 `std::optional<T>`.

---

## 5. Generator Functions → Eager `std::vector` ([_loop.hpp](rich/_loop.hpp))

**Python Behavior:**
`_loop.py` uses generator functions (`loop_first`, `loop_last`, `loop_first_last`) to lazily yield element position metadata.

**C++ Decision:**
To maintain C++17 compatibility without adding C++20 coroutine requirements, functions return materialized `std::vector<std::tuple<...>>` containers.

---

## 6. Codepoint-Indexed Strings → `std::u32string` ([cells.hpp](rich/cells.hpp))

**Python Behavior:**
Python strings index by Unicode codepoints (`str` indexing).

**C++ Decision:**
C++ `std::string` stores UTF-8 bytes where string length and indexing operate per-byte. Modules requiring codepoint-accurate indexing ([cells.hpp](rich/cells.hpp)) operate on UTF-32 (`std::u32string`) internally, converting to/from UTF-8 `std::string` at API boundaries via standard conversion utilities.

---

## 7. Keyword Arguments → Builder Pattern ([style.hpp](rich/style.hpp))

**Python Behavior:**
`Style(bold=True, color="red")` uses arbitrary keyword arguments.

**C++ Decision:**
`Style` uses chainable method setters (`Style{}.set_bold().set_color("red")`) and string parsing (`Style::parse("bold red")`).

---

## 8. Performance Caching (`@lru_cache`)

**Python Behavior:**
Python Rich uses `@lru_cache` on frequent style parsing calls.

**C++ Decision:**
Caching is currently omitted in the core port. `Style::parse()` and `Color::parse()` evaluate string inputs on each call.

---

## 9. Explicit Scope Cuts

- **`Color::downgrade()`**: Truecolor to 8-bit / 4-bit color degradation is deferred until palette tables are extracted.
- **Console Line Wrapping**: Width-aware line wrapping is deferred until `Text` and `Segment` components are implemented.
- **Custom Box Styles**: Tables and panels currently use default box-drawing character sets (heavy head for tables, rounded corners for panels).
