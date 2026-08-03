# Porting Decisions: Python → C++

This document records technical design decisions, architectural divergences, and rationale made during the port of [Textualize/rich](https://github.com/Textualize/rich) from Python to C++17 for **Port Mortem 2026**.

---

## 1. Duck-Typing → Compile-Time SFINAE Traits ([protocol.hpp](src/rich/protocol.hpp), [abc.hpp](src/rich/abc.hpp))

**What changed:**
Replaced runtime subclass checking (`isinstance(obj, RichRenderable)`) with compile-time SFINAE detection traits (`has_rich_method<T>`, `is_rich_renderable_v<T>`).

**Why:**
C++ lacks dynamic runtime duck-typing without forced inheritance hierarchies. Using SFINAE traits (`std::void_t`) allows non-intrusive rendering of third-party C++ types without performance or virtual table overhead.

**Alternatives considered:**
Relying on a common polymorphic base class `IRenderable` with virtual methods. Rejected because header-only C++ libraries benefit from static polymorphism and zero-cost abstraction.

---

## 2. `NamedTuple` → Plain C++ Structs ([color_triplet.hpp](src/rich/color_triplet.hpp), [region.hpp](src/rich/region.hpp))

**What changed:**
Mapped Python `NamedTuple` containers (`ColorTriplet`, `Region`) to lightweight C++ `struct` types with explicit `operator==` implementations.

**Why:**
C++ `struct` provides POD layout, low overhead, and straightforward field access (`color.r`, `region.x`).

**Alternatives considered:**
`std::tuple` or `std::pair`. Rejected because named struct members (`.r`, `.g`, `.b`) maintain exact field name parity with Python `NamedTuple`.

---

## 3. Exceptions Hierarchy ([errors.hpp](src/rich/errors.hpp))

**What changed:**
Derived custom exception classes (`rich::RichError`, `rich::StyleSyntaxError`) from `std::runtime_error`.

**Why:**
Standard C++ exception handling relies on type-based `catch` blocks deriving from `std::exception`.

**Alternatives considered:**
Returning error codes or `std::expected`. Rejected to match Python's exception propagation model for invalid markup or style parsing.

---

## 4. Codepoint-Indexed Strings → `std::u32string` ([cells.hpp](src/rich/cells.hpp))

**What changed:**
Converted UTF-8 byte strings (`std::string`) to UTF-32 (`std::u32string`) inside text width calculation and grapheme splitting routines.

**Why:**
Python strings index by Unicode codepoints (`str` indexing), whereas C++ `std::string` length and indexing operate on raw UTF-8 bytes. UTF-32 ensures 1-to-1 codepoint indexing for terminal cell width measurement.

**Alternatives considered:**
Operating directly on raw byte offsets. Rejected because multi-byte UTF-8 sequences and CJK wide characters produce invalid cell lengths when indexed by byte offset.

---

## 5. Generator Functions → Materialized `std::vector` ([_loop.hpp](src/rich/_loop.hpp))

**What changed:**
Replaced lazy Python generator functions (`loop_first`, `loop_last`) with functions returning materialized `std::vector<std::tuple<...>>`.

**Why:**
Maintains standard C++17 compatibility without requiring C++20 coroutines or third-party generator libraries.

**Alternatives considered:**
Custom iterator wrapper state machines. Rejected due to increased complexity for simple iteration helpers.

---

## 6. Keyword Arguments → Chainable Builder Pattern ([style.hpp](src/rich/style.hpp))

**What changed:**
Replaced Python keyword arguments (`Style(bold=True, color="red")`) with chainable setter methods (`Style{}.set_bold().set_color("red")`) and string parsing (`Style::parse("bold red")`).

**Why:**
C++ standard syntax does not support arbitrary named keyword arguments in constructors.

**Alternatives considered:**
Struct initializer parameter lists (`Style{ .bold = true, .color = ... }`). Rejected for C++17 header compatibility.

---

## 7. Fuzzing & Test Divergence Findings

**What changed:**
No behavioral divergences were found during 60+ seconds of differential fuzzing against Python `rich`.

**Why:**
All cell width calculations, ANSI escape sequence generation, and border drawing logic match Python `rich` output byte-for-byte.

**Alternatives considered:**
N/A.
