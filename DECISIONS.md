# Porting Decisions: Python → C++


## 1. Duck-Typing → Compile-Time SFINAE Traits ([src/rich/protocol.hpp](src/rich/protocol.hpp), [src/rich/abc.hpp](src/rich/abc.hpp))

**What we changed:**
Replaced runtime subclass checking (`isinstance(obj, RichRenderable)`) with compile-time SFINAE detection traits (`has_rich_method<T>`, `is_rich_renderable_v<T>`).

**Why ?:**
C++ lacks dynamic runtime duck-typing without forced inheritance hierarchies. Using SFINAE traits (`std::void_t`) allows non-intrusive rendering of third-party C++ types without performance or virtual table overhead.

**Alternatives considered:**
Relying on a common polymorphic base class `IRenderable` with virtual methods. Rejected because header-only C++ libraries benefit from static polymorphism and zero-cost abstraction.

---

## 2. `NamedTuple` → Plain C++ Structs ([src/rich/color_triplet.hpp](src/rich/color_triplet.hpp), [src/rich/region.hpp](src/rich/region.hpp))

**What we changed:**
Mapped Python `NamedTuple` containers (`ColorTriplet`, `Region`) to lightweight C++ `struct` types with explicit `operator==` implementations.

**Why ?:**
C++ `struct` provides POD layout, low overhead, and straightforward field access (`color.r`, `region.x`).

**Alternatives considered:**
`std::tuple` or `std::pair`. Rejected because named struct members (`.r`, `.g`, `.b`) maintain exact field name parity with Python `NamedTuple`.

---

## 3. Exceptions Hierarchy ([src/rich/errors.hpp](src/rich/errors.hpp))

**What we changed:**
Derived custom exception classes (`rich::RichError`, `rich::StyleSyntaxError`, `rich::ColorParseError`) from `std::runtime_error`.

**Why ?:**
Standard C++ exception handling relies on type-based `catch` blocks deriving from `std::exception`.

**Alternatives considered:**
Returning error codes or `std::expected`. Rejected to match Python's exception propagation model for invalid markup or style parsing.

---

## 4. Codepoint-Indexed Strings → `std::u32string` ([src/rich/cells.hpp](src/rich/cells.hpp))

**What we changed:**
Converted UTF-8 byte strings (`std::string`) to UTF-32 (`std::u32string`) inside text width calculation and grapheme splitting routines.

**Why ?:**
Python strings index by Unicode codepoints (`str` indexing), whereas C++ `std::string` length and indexing operate on raw UTF-8 bytes. UTF-32 ensures 1-to-1 codepoint indexing for terminal cell width measurement.

**Alternatives considered:**
Operating directly on raw byte offsets. Rejected because multi-byte UTF-8 sequences and CJK wide characters produce invalid cell lengths when indexed by byte offset.

---

## 5. Generator Functions → Materialized `std::vector` ([src/rich/_loop.hpp](src/rich/_loop.hpp))

**What we changed:**
Replaced lazy Python generator functions (`loop_first`, `loop_last`) with functions returning materialized `std::vector<std::tuple<...>>`.

**Why ?:**
Maintains standard C++17 compatibility without requiring C++20 coroutines or third-party generator libraries.

**Alternatives considered:**
Custom iterator wrapper state machines. Rejected due to increased complexity for simple iteration helpers.

---

## 6. Keyword Arguments → Chainable Builder Pattern ([src/rich/style.hpp](src/rich/style.hpp))

**What we changed:**
Replaced Python keyword arguments (`Style(bold=True, color="red")`) with chainable setter methods (`Style{}.set_bold().set_color("red")`) and string parsing (`Style::parse("bold red")`).

**Why ?:**
C++ standard syntax does not support arbitrary named keyword arguments in constructors.

**Alternatives considered:**
Struct initializer parameter lists (`Style{ .bold = true, .color = ... }`). Rejected for C++17 header compatibility.

---

## 7. Fuzzing & Test Divergence Findings

**What we changed:**
Zero behavioral divergences were found during differential fuzzing against Python `rich` (`fuzz/harness.py`).

**Why ?:**
All cell width calculations, ANSI escape sequence generation, and border drawing logic match Python `rich` output byte-for-byte.

**Alternatives considered:**
N/A.
