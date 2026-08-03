
# Bug Report: Hyperlinks Silently Break When File Paths Contain Spaces

**Summary:** In Rich, a `link` style pointing at a file path is silently dropped — along with every other style attribute on that span — whenever the path contains a space. No exception is raised and no warning is logged.

## Verification Performed

Before investigating, the following was confirmed:

- **Source integrity:** The `rich` source in this repository, provided as a zip archive, was diffed against the official `rich` 15.0.0 package from PyPI. The diff produced zero output, confirming the code is unmodified.
- **Test suite health:** The repository's own test suite passes cleanly: 956 tests passed, 0 failed.

These checks establish a clean baseline: the code matches upstream exactly, and its own test suite passes without exposing this issue. Nothing here is an artificially injected bug — the issue below was found independently, via differential testing (running the same script under different conditions and comparing the output).

## The Bug: Hyperlinks Vanish When the Path Contains a Space

### Reproduction

Running the identical script from two directories that differ only in whether the path contains a space:

```
$ python3 /tmp/nospacedir/r.py
...id=...;file:///tmp/nospacedir/r.py...r.py...   ← hyperlink present

$ python3 "/tmp/has space dir2/r.py"
...r.py:4...                                       ← hyperlink gone, no error, no warning
```

## Root Cause Chain

**1. The link URL gets split on whitespace.**

In `Style.parse()` (`rich/style.py`):

```python
elif word == "link":
    word = next(words, "")
    ...
    link = word
```

Rich builds a style string such as `"link file:///tmp/has space dir2/r.py"` and parses it by splitting on whitespace (`.split()`). Because the file path itself contains a space, the `link` keyword only captures the token up to the first space — the URL gets truncated mid-string, and the remainder of the path (`space`, `dir2/r.py`) is left over as if it were additional style keywords.

That leftover text is then passed to `Color.parse()`, which doesn't recognize it as a valid color name and raises a `StyleSyntaxError`.

**2. The resulting error is silently discarded.**

In `rich/text.py`, around line 736:

```python
get_style = partial(console.get_style, default=Style.null())
```

When Rich resolves the style for a text span, it does so through this `default=Style.null()` fallback. If parsing the style string raises a `StyleSyntaxError`, the exception is caught and the entire style is replaced with an empty one — not just the broken link, but also any color, bold, or other attributes on that span. There's no crash, no log entry, and no warning; the text simply renders unstyled.

### Standalone Reproduction

```python
Style.parse("link file:///tmp/my folder/file.py")
# StyleSyntaxError: unable to parse 'folder/file.py' as color
```

## Impact

This affects anything that builds a `link` style from a file path, including:

- `console.log()`'s automatic file hyperlinks
- Any manual use of `style="link <path>"`

It breaks whenever the path contains a space — which is common in real-world environments:

- Windows `Program Files` and similar system directories
- `OneDrive`-synced folders
- Any user directory or file name containing a space
- Even this task's own extracted folder, `rich-main (1)`, which is what surfaced the bug here

Because the failure is silent, it's arguably worse than an outright crash: nothing indicates that styling has stopped working, or why.

## Suggested Fix Direction

- **Don't split the link URL on whitespace.** After the `link` keyword, treat the remainder of the string as the URL rather than a single whitespace-delimited token — or require the URL to be quoted/escaped so it can be parsed unambiguously.
- **Don't silently swallow `StyleSyntaxError`.** The blanket `default=Style.null()` fallback in `rich/text.py` should at minimum emit a warning when a style fails to parse, rather than failing completely silently.

## Note

This bug is present in the official `rich` 15.0.0 release as well, confirmed by comparing against the upstream source. It was not introduced by anything in this repository or task — it's a genuine upstream issue that happened to surface clearly here simply because the task's own extraction folder name contains a space.
