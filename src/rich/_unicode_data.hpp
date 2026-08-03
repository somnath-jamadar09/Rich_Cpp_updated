// Port of rich/_unicode_data/__init__.py
//
// SCOPE NOTE: the Python original supports 22 selectable Unicode versions
// (4.1.0 through 17.0.0) via `UNICODE_VERSION` env var / `unicode_version`
// param, resolved with bisect against a sorted version list. This port only
// ships the "latest" (17.0.0) table (see _unicode_data_17_0_0.hpp, which is
// mechanically extracted real data, not fabricated). `load()` here accepts
// "auto" / "latest" and any other string falls back to latest, with a
// warning path left as a TODO — porting the other 21 version tables is
// listed in the README roadmap.
#pragma once
#include "_unicode_data_17_0_0.hpp"
#include <string>

namespace rich::unicode_data {

struct CellTable {
    const char* unicode_version;
    const decltype(kWidths)& widths;
    // Membership test replaces Python's frozenset<str>.
    bool is_narrow_to_wide(const std::u32string& s) const {
        return narrow_to_wide().count(s) > 0;
    }
};

/// Load a cell table for the given unicode version.
/// Only "auto", "latest", or "17.0.0" resolve to real data in this port.
inline const CellTable& load(const std::string& /*unicode_version*/ = "auto") {
    static const CellTable table{kUnicodeVersion, kWidths};
    return table;
}

} // namespace rich::unicode_data
