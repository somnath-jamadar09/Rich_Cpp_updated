# Porting decisions: Python → C++

This document records every design decision made while porting Rich from
Python to C++ — specifically the places where a straight line-by-line
translation was impossible and a real engineering choice had to be made.
Mechanical 1:1 translations (e.g. `if`/`for`/basic arithmetic) aren't
listed; only the decisions that changed behavior, structure, or scope are.

## 1. Duck-typing → compile-time traits (`protocol.py`, `abc.py`)

**Python does:** `isinstance(obj, RichRenderable)` succeeds for *any*
object that merely has a `__rich__` or `__rich_console__` method, checked
at runtime via `ABCMeta.__subclasshook__` — no inheritance required.

**Problem:** C++ has no runtime structural type check. `isinstance()`
requires either explicit inheritance (which would force every renderable
type in a user's codebase to inherit from a Rich base class — un-Pythonic
and impractical) or `dynamic_cast` (which still requires a common base).

**Decision:** Use the C++ "detection idiom" — SFINAE traits
(`has_rich_method<T>`, `has_rich_console_method<T>`) resolved at compile
time via `std::void_t`. `rich::is_rich_renderable_v<T>` replaces
`isinstance(obj, RichRenderable)`, checked with `if constexpr` instead of
a runtime `if`.

**Trade-off accepted:** Type-checking moves from runtime to compile time.
This is *stricter* than Python (a template won't compile for a type
without the right method, vs. Python raising at call time) but loses the
ability to check an unknown type dynamically at runtime (e.g. from a
plugin loaded via `dlopen`). For a terminal-formatting library this
trade-off is acceptable — nobody needs runtime plugin renderables.

## 2. `NamedTuple` → `struct`

**Python does:** `Color`, `ColorTriplet`, `Region` are `NamedTuple`
subclasses — immutable, structurally-compared, unpackable.

**Decision:** Plain C++ `struct` with public fields. `operator==` added by
hand where Python's structural equality was actually used (`ColorTriplet`).
No immutability enforced (Python's `NamedTuple` immutability is
convention-level in practice, not a hard guarantee either).

## 3. Exceptions

**Python does:** A flat hierarchy of `Exception` subclasses in
`errors.py`, each just a docstring + default message.

**Decision:** Mirror the exact hierarchy with `std::runtime_error`
subclasses, same names, same default messages. Kept the inheritance
relationships identical (e.g. `StyleSyntaxError` and `MissingStyle` both
derive from `ConsoleError`'s Python equivalent chain) so `catch` blocks
written against a base class behave the same way in both languages.

## 4. `Optional[bool]` and similar → `std::optional<T>`

Direct mapping, no decision needed beyond confirming `std::optional`
(C++17) covers every case Python's `Optional` was used for in the ported
files. This is why C++17 (not C++11/14) was the minimum target.

## 5. Generators → eager `std::vector`

**Python does:** `_loop.py`'s `loop_first`/`loop_last`/`loop_first_last`
are generator functions — lazy, one item at a time.

**Decision:** Return a fully-materialized `std::vector` of tuples instead
of implementing a lazy C++ generator/coroutine. Coroutines (C++20) would
be the closer match, but the project targets C++17 for wider compiler
compatibility, and these helper functions are only ever used over small,
already-in-memory containers in Rich — laziness has no real payoff here.

## 6. Python strings (codepoint sequences) → `std::u32string`

**Problem:** Python `str` is a sequence of Unicode codepoints; indices,
slicing, and length all operate per-codepoint. C++ `std::string` is a
byte sequence (UTF-8 assumed), where indices/length are per-*byte* —
completely different semantics for any non-ASCII text.

**Decision:** Any module whose logic depends on per-codepoint indexing
(`cells.py`'s grapheme splitting, width measurement, cropping) operates on
`std::u32string` (`char32_t` per element) internally, converting from/to
UTF-8 `std::string` only at API boundaries (via `std::wstring_convert` +
`std::codecvt_utf8<char32_t>`). This keeps index arithmetic identical to
the Python original instead of requiring a UTF-8-aware reimplementation of
every offset calculation.

**Known caveat:** `std::wstring_convert`/`std::codecvt_utf8` are
deprecated in C++17 (removal proposed for a future standard, no
replacement shipped yet). Kept anyway since there's no adopted stdlib
replacement as of C++17/20; a future revision should switch to a small
hand-rolled UTF-8 codec or a vetted third-party one if this leaves
header-only/zero-dependency territory.

## 7. Keyword arguments → builder-style setters (`style.hpp`)

**Python does:** `Style(bold=True, color="red")` — arbitrary keyword
arguments, all optional, order-independent.

**Decision:** C++ has no named parameters. Used chainable setters instead:
`Style{}.set_bold().set_color("red")`. Chosen over (a) a struct-of-optionals
passed by value (more verbose at call sites) or (b) a variadic/tag-based
emulation of kwargs (needless complexity for the actual usage pattern,
which is almost always `Style::parse("bold red")` from a string anyway).

## 8. `lru_cache` decorators — dropped, not ported

**Python does:** `Color.parse`, `Style` internals, etc. are wrapped in
`@lru_cache` for performance, since these are called very frequently with
repeated arguments (e.g. the same style string parsed thousands of times
during a render).

**Decision:** Not ported in this pass. C++ `Color::parse`/`Style::parse`
recompute every call. This is a correctness-neutral, performance-only gap
— acceptable for the current scope (a demo/hackathon-stage port), but
flagged here because a production port would want an
`unordered_map<string, Style>`-based memoization layer before this is used
in a hot rendering loop.

## 9. Scope cuts: what was *not* ported, and why

- **`Color::downgrade()`** — converts truecolor → 8-bit → 16-color for
  terminals that don't support 16.7M colors. Needs `_palettes.py`'s
  `EIGHT_BIT_PALETTE`/`STANDARD_PALETTE`/`WINDOWS_PALETTE` (large
  generated tables) — not yet extracted. Every ported `Style`/`Color`
  currently emits full truecolor/8-bit codes unconditionally; fine on any
  modern terminal, wrong on a legacy 16-color one.
- **`Console`'s width-aware wrapping/justify** — needs `Text`/`Segment`
  (not yet ported) for grapheme-aware line-splitting. Current
  `console.hpp` prints unwrapped.
- **Table/Panel styling, multi-line content, custom box styles** — the
  ported subset hard-codes Rich's *default* box style and single-line
  content, matching the common case exactly (verified byte-for-byte) but
  not the full parameter surface of the Python classes.

Each of these is a deliberate scope cut for this pass, not an oversight —
tracked in [WORKFLOW.md](WORKFLOW.md)'s roadmap.
