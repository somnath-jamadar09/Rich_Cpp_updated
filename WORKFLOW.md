# Workflow: how this port was built and verified

## The process, per module

Every ported file went through the same 5 steps — this is the repeatable
process for continuing the port, not a one-off:

1. **Read the real source.** Open the actual `rich/<name>.py` from the
   original repo and read the logic being ported — not from memory/guess,
   from the actual file in this snapshot.
2. **Measure scope.** Note line count and dependencies before starting —
   decide whether the *whole* file is portable now or only a *core subset*
   (e.g. `color.py` is 621 lines but only ~150 of them — parsing +
   ANSI-code generation — were in scope for this pass; `downgrade()` was
   explicitly deferred, see DECISIONS.md).
3. **Translate, not transliterate.** Where Python idioms have no direct
   C++ equivalent (duck-typing, keyword args, generators, codepoint-indexed
   strings), make a real design decision instead of a broken literal
   translation — every such decision is logged in DECISIONS.md as it's
   made, not after the fact.
4. **Compile-check immediately.** `g++ -std=c++17 -I. <test>.cpp -o
   <test>` after every file, before moving to the next one. A file that
   doesn't compile doesn't count as done.
5. **Verify against live Python where behavior is observable.** For any
   function/class with externally-checkable output (not just internal
   plumbing), install the real package (`pip install -e .` from the
   original source) in the same sandbox, run the *actual* Python function
   on a test input, capture its exact output, then run the C++ port on the
   same input and diff the two — byte-for-byte, not "looks similar".

## Verification log (what was actually checked, and how)

| Module | Check performed | Result |
|---|---|---|
| `cells.py` → `cells.hpp` | `cell_len()`, `chop_cells()`, `set_cell_size()` run on ASCII / CJK (`你好`) / emoji (`😀`) inputs in both languages, outputs compared | Exact match, all cases |
| `style.py` → `style.hpp` | `Style.parse("bold red on blue").render("hi")` in both languages | `\x1b[1;31;44mhi\x1b[0m` in both — byte-identical |
| `console.py` (markup) → `console.hpp` | Nested markup `"[bold]a [red]b[/red] c[/bold]"` printed via both `rich.console.Console.print()` and C++ `print_markup()` | Byte-identical ANSI sequence in both |
| `rule.py` → `rule.hpp` | `console.rule("Title")` at width=40 in both | Byte-identical, including exact dash counts (16 left / 17 right) |
| `panel.py` → `panel.hpp` | `Panel("Hi there")` at width=20 in both | Byte-identical, including exact box-drawing codepoints |
| `table.py` → `table.hpp` | 2-column/2-row/titled table, both languages, output written to files and `diff`'d | `diff` reports zero differences |

Data-table extractions (not "verified" in the behavioral sense above, but
sourced mechanically rather than hand-typed, which is its own form of
correctness guarantee):

| Data | Extraction method |
|---|---|
| Unicode width table (464 ranges) | `ast.parse()` + `ast.literal_eval()` on the real `unicode17-0-0.py`, dumped to JSON, then rendered into a C++ header by a small generator script — no number was manually typed |
| ANSI color names (235 entries) | Same `ast.literal_eval()` approach on `color.py`'s `ANSI_COLOR_NAMES` dict |

## Reproducing the verification yourself

```bash
# from the original Python rich source tree
pip install -e . --break-system-packages

python3 -c "
from rich.style import Style
print(repr(Style.parse('bold red on blue').render('hi')))
"
# compare against the C++ output of the equivalent test in style.hpp
```

Each verification above followed this same pattern: real Python call →
captured `repr()` of the output → C++ test program producing the same
input → string comparison in a small throwaway test `.cpp` file (not
checked into the repo — the *result* of the verification is what's
recorded here and in README.md's status table).

## Why 16 of 77 files, not all of them

The Python package is 77 files / ~38,500 lines. Verifying at the standard
above (step 5, byte-for-byte against live output) is what makes each
ported file trustworthy — but it's also why coverage is incremental rather
than "convert everything in one pass". A full-package pass at the same
rigor is a genuinely large effort; each session has extended real,
verified coverage rather than filling gaps with unverified/guessed
translations. See DECISIONS.md §9 for exactly which sub-features within
already-ported files were deliberately deferred.

## Roadmap (dependency order — matches ARCHITECTURE.md's layers)

1. **Layer 1 remainder**: `box.py`, `_palettes.py`, `default_styles.py`,
   `palette.py`, `theme.py`, `themes.py` — needed for `Color::downgrade()`
   and swappable box styles.
2. **Layer 2 remainder**: `segment.py`, `text.py` — `Segment`
   (styled-text-span primitive) and `Text` (word-wrap/justify), unlocking
   real width-aware `Console` output.
3. **Layer 3 completion**: the remaining ~2650 lines of `console.py` —
   full render protocol, `Live`, capturing/recording output.
4. **Layer 4 expansion**: `align.py`, `columns.py`, `padding.py`,
   `tree.py`, `layout.py`, `live.py`; broaden `table.py`/`panel.py` beyond
   their current default-style/single-line subset.
5. **Content modules**: `markdown.py`, `syntax.py`, `pretty.py`,
   `traceback.py`, `progress.py`, `json.py`, `logging.py` — these depend
   on everything above being in place first.
6. **Remaining data tables**: `_emoji_codes.py` (3610 lines, pure data —
   mechanical extraction like the two already done), `_spinners.py`,
   `_emoji_replace.py`.

Each future addition should follow the same 5-step process and get the
same line-item treatment in README.md's status table, DECISIONS.md (if a
real design choice was needed), and this file's verification log.
