// Port of rich/cells.py
//
// Python strings are sequences of Unicode codepoints; the closest C++
// equivalent for a faithful, index-compatible port is std::u32string
// (char32_t per codepoint), used throughout this file instead of
// std::string (UTF-8, variable width, indices wouldn't match).
#pragma once
#include "_unicode_data.hpp"
#include <string>
#include <vector>
#include <tuple>
#include <array>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace rich {

/// (start, end, cell_length)
using CellSpan = std::tuple<int, int, int>;

namespace detail_cells {

// Ranges of unicode ordinals that produce a 1-cell wide character.
// Non-exhaustive, but covers most common Western characters.
inline constexpr std::array<std::pair<char32_t, char32_t>, 6> kSingleCellRanges = {{
    {0x20, 0x7E},      // Latin (excluding non-printable)
    {0xA0, 0xAC},
    {0xAE, 0x002FF},
    {0x00370, 0x00482}, // Greek / Cyrillic
    {0x02500, 0x025FC}, // Box drawing, box elements, geometric shapes
    {0x02800, 0x028FF}, // Braille
}};

inline bool is_single_cell_char(char32_t c) {
    for (auto& [start, end] : kSingleCellRanges) {
        if (c >= start && c <= end) return true;
    }
    return false;
}

inline bool is_single_cell_widths(const std::u32string& text) {
    for (char32_t c : text) {
        if (!is_single_cell_char(c)) return false;
    }
    return true;
}

constexpr char32_t ZWJ = 0x200D;  // zero width joiner
constexpr char32_t VS16 = 0xFE0F; // variation selector 16

} // namespace detail_cells

/// Get the cell size of a character.
/// Returns: Number of cells (0, 1 or 2) occupied by that character.
inline int get_character_cell_size(char32_t character,
                                    const std::string& unicode_version = "auto") {
    int32_t codepoint = static_cast<int32_t>(character);
    if ((codepoint && codepoint < 32) || (codepoint >= 0x07F && codepoint < 0x0A0)) {
        return 0;
    }
    const auto& table = unicode_data::load(unicode_version);
    const auto& widths = table.widths;

    const auto& last_entry = widths.back();
    if (codepoint > last_entry.end) {
        return 1;
    }

    int lower_bound = 0;
    int upper_bound = static_cast<int>(widths.size()) - 1;
    while (lower_bound <= upper_bound) {
        int index = (lower_bound + upper_bound) >> 1;
        const auto& [start, end, width] = widths[index];
        if (codepoint < start) {
            upper_bound = index - 1;
        } else if (codepoint > end) {
            lower_bound = index + 1;
        } else {
            return width;
        }
    }
    return 1;
}

/// Get the cell length of a string (length as it appears in the terminal).
inline int cell_len(const std::u32string& text, const std::string& unicode_version = "auto") {
    if (detail_cells::is_single_cell_widths(text)) {
        return static_cast<int>(text.size());
    }

    bool has_special = text.find(detail_cells::ZWJ) != std::u32string::npos ||
                        text.find(detail_cells::VS16) != std::u32string::npos;
    if (!has_special) {
        int total = 0;
        for (char32_t c : text) total += get_character_cell_size(c, unicode_version);
        return total;
    }

    const auto& cell_table = unicode_data::load(unicode_version);
    int total_width = 0;
    char32_t last_measured_character = 0;
    bool have_last = false;

    size_t index = 0;
    size_t n = text.size();
    while (index < n) {
        char32_t character = text[index];
        if (character == detail_cells::ZWJ || character == detail_cells::VS16) {
            if (character == detail_cells::ZWJ) {
                index += 1;
            } else if (have_last) {
                std::u32string single(1, last_measured_character);
                if (cell_table.is_narrow_to_wide(single)) total_width += 1;
                have_last = false;
            }
        } else {
            int character_width = get_character_cell_size(character, unicode_version);
            if (character_width) {
                last_measured_character = character;
                have_last = true;
                total_width += character_width;
            }
        }
        index += 1;
    }
    return total_width;
}

/// Divide text into spans that define a single grapheme, and additionally
/// return the cell length of the whole string.
inline std::pair<std::vector<CellSpan>, int> split_graphemes(
    const std::u32string& text, const std::string& unicode_version = "auto") {
    const auto& cell_table = unicode_data::load(unicode_version);
    size_t codepoint_count = text.size();
    size_t index = 0;
    char32_t last_measured_character = 0;
    bool have_last = false;

    int total_width = 0;
    std::vector<CellSpan> spans;

    while (index < codepoint_count) {
        char32_t character = text[index];
        if (character == detail_cells::ZWJ || character == detail_cells::VS16) {
            if (spans.empty()) {
                spans.emplace_back((int)index, (int)(index + 1), 0);
                index += 1;
                continue;
            }
            if (character == detail_cells::ZWJ) {
                index += (index < codepoint_count - 1) ? 2 : 1;
                auto [start, end, cell_length] = spans.back();
                spans.back() = {start, (int)index, cell_length};
            } else {
                index += 1;
                if (have_last) {
                    auto [start, end, cell_length] = spans.back();
                    std::u32string single(1, last_measured_character);
                    if (cell_table.is_narrow_to_wide(single)) {
                        have_last = false;
                        cell_length += 1;
                        total_width += 1;
                    }
                    spans.back() = {start, (int)index, cell_length};
                } else {
                    auto [start, end, cell_length] = spans.back();
                    spans.back() = {start, (int)index, cell_length};
                }
            }
            continue;
        }

        int character_width = get_character_cell_size(character, unicode_version);
        if (character_width) {
            last_measured_character = character;
            have_last = true;
            spans.emplace_back((int)index, (int)(index + 1), character_width);
            total_width += character_width;
            index += 1;
        } else {
            if (!spans.empty()) {
                auto [start, end, cell_length] = spans.back();
                index += 1;
                spans.back() = {start, (int)index, cell_length};
            } else {
                spans.emplace_back((int)index, (int)(index + 1), 0);
                index += 1;
            }
        }
    }
    return {spans, total_width};
}

namespace detail_cells {
inline std::pair<std::u32string, std::u32string> split_text_impl(
    const std::u32string& text, int cell_position, const std::string& unicode_version) {
    if (cell_position <= 0) return {U"", text};

    auto [spans, cell_length] = split_graphemes(text, unicode_version);
    if (cell_length == 0) return {text, U""};

    int offset = static_cast<int>((static_cast<double>(cell_position) / cell_length) *
                                   spans.size());
    offset = std::clamp(offset, 0, (int)spans.size());
    int left_size = 0;
    for (int i = 0; i < offset; ++i) left_size += std::get<2>(spans[i]);

    while (true) {
        if (left_size == cell_position) {
            if (offset >= (int)spans.size()) return {text, U""};
            int split_index = std::get<0>(spans[offset]);
            return {text.substr(0, split_index), text.substr(split_index)};
        }
        if (left_size < cell_position) {
            auto [start, end, cell_size] = spans[offset];
            if (left_size + cell_size > cell_position) {
                return {text.substr(0, start) + U" ", U" " + text.substr(end)};
            }
            offset += 1;
            left_size += cell_size;
        } else {
            auto [start, end, cell_size] = spans[offset - 1];
            if (left_size - cell_size < cell_position) {
                return {text.substr(0, start) + U" ", U" " + text.substr(end)};
            }
            offset -= 1;
            left_size -= cell_size;
        }
    }
}
} // namespace detail_cells

/// Split text by cell position. If the cell position falls within a double
/// width character, it is converted to two spaces.
inline std::pair<std::u32string, std::u32string> split_text(
    const std::u32string& text, int cell_position,
    const std::string& unicode_version = "auto") {
    if (detail_cells::is_single_cell_widths(text)) {
        int pos = std::clamp(cell_position, 0, (int)text.size());
        return {text.substr(0, pos), text.substr(pos)};
    }
    return detail_cells::split_text_impl(text, cell_position, unicode_version);
}

/// Adjust a string by cropping or padding with spaces such that it fits
/// within the given number of cells.
inline std::u32string set_cell_size(const std::u32string& text, int total,
                                     const std::string& unicode_version = "auto") {
    if (detail_cells::is_single_cell_widths(text)) {
        int size = static_cast<int>(text.size());
        if (size < total) return text + std::u32string(total - size, U' ');
        return text.substr(0, std::max(0, total));
    }
    if (total <= 0) return U"";
    int cell_size = cell_len(text, unicode_version);
    if (cell_size == total) return text;
    if (cell_size < total) return text + std::u32string(total - cell_size, U' ');
    return split_text(text, total, unicode_version).first;
}

/// Split text into lines such that each line fits within the available
/// (cell) width.
inline std::vector<std::u32string> chop_cells(const std::u32string& text, int width,
                                               const std::string& unicode_version = "auto") {
    std::vector<std::u32string> lines;
    if (detail_cells::is_single_cell_widths(text)) {
        for (size_t i = 0; i < text.size(); i += width) {
            lines.push_back(text.substr(i, width));
        }
        return lines;
    }
    auto [spans, _] = split_graphemes(text, unicode_version);
    int line_size = 0;
    size_t line_offset = 0;
    for (auto& [start, end, cell_size] : spans) {
        if (line_size + cell_size > width) {
            lines.push_back(text.substr(line_offset, start - line_offset));
            line_offset = start;
            line_size = 0;
        }
        line_size += cell_size;
    }
    if (line_size) {
        lines.push_back(text.substr(line_offset));
    }
    return lines;
}

} // namespace rich
