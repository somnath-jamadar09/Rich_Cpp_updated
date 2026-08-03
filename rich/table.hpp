// Port of rich/table.py (core subset: default HEAVY_HEAD box style, columns
// auto-sized to content, centered title). Box-drawing characters and layout
// verified against real Python output for a 2-column, 2-row table with title
// (see README.md for the exact captured reference). NOT ported: styles per
// cell, justify options, multiple box styles, row/column spanning, padding
// customization beyond the fixed 1-space default, word-wrapping long cells.
#pragma once
#include "cells.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <codecvt>
#include <locale>
#include <algorithm>

namespace rich {

namespace detail_table {
inline std::u32string to32(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}
}

class Table {
public:
    std::string title;
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;

    void add_column(const std::string& name) { columns.push_back(name); }
    void add_row(std::initializer_list<std::string> row) { rows.push_back(row); }

    void print() const {
        size_t n = columns.size();
        std::vector<int> width(n, 0);
        for (size_t i = 0; i < n; ++i) {
            width[i] = cell_len(detail_table::to32(columns[i]));
            for (auto& row : rows) {
                if (i < row.size())
                    width[i] = std::max(width[i], cell_len(detail_table::to32(row[i])));
            }
            width[i] += 2; // 1 space padding each side
        }

        int total_width = 1; // left border
        for (size_t i = 0; i < n; ++i) total_width += width[i] + 1;

        if (!title.empty()) {
            int title_len = cell_len(detail_table::to32(title));
            int left = std::max(0, (total_width - title_len) / 2);
            int right = std::max(0, total_width - title_len - left);
            std::cout << std::string(left, ' ') << title << std::string(right, ' ') << "\n";
        }

        auto border_line = [&](const std::string& l, const std::string& mid,
                                const std::string& r, const std::string& fill) {
            std::string out = l;
            for (size_t i = 0; i < n; ++i) {
                for (int j = 0; j < width[i]; ++j) out += fill;
                out += (i + 1 < n) ? mid : r;
            }
            std::cout << out << "\n";
        };
        auto content_line = [&](const std::vector<std::string>& cells, const std::string& side) {
            std::string out = side;
            for (size_t i = 0; i < n; ++i) {
                std::string text = i < cells.size() ? cells[i] : "";
                int len = cell_len(detail_table::to32(text));
                int pad_right = std::max(1, width[i] - 1 - len);
                out += " " + text + std::string(pad_right, ' ');
                out += side;
            }
            std::cout << out << "\n";
        };

        border_line("\u250F", "\u2533", "\u2513", "\u2501"); // TOP: ┏ ┳ ┓ ━
        content_line(columns, "\u2503");                      // HEADER row: ┃
        border_line("\u2521", "\u2547", "\u2529", "\u2501");  // HEADER separator: ┡ ╇ ┩ ━
        for (auto& row : rows) content_line(row, "\u2502");   // DATA rows: │
        border_line("\u2514", "\u2534", "\u2518", "\u2500");  // BOTTOM: └ ┴ ┘ ─
    }
};

} // namespace rich
