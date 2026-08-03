// Port of rich/panel.py (core subset: default ROUNDED box style, fixed
// width, single-line content, no title). Verified against real Python
// output: Panel("Hi there") at width=20 renders:
//   ╭──────────────────╮
//   │ Hi there         │
//   ╰──────────────────╯
// NOT ported yet: title, subtitle, custom box styles, auto width from
// content, padding options, multi-line/wrapped content (needs Text).
#pragma once
#include "cells.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <codecvt>
#include <locale>

namespace rich {

namespace detail_panel {
inline std::u32string to32(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}
inline std::string to8(const std::u32string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}
}

class Panel {
public:
    static void print(const std::string& content, int width = -1) {
        std::u32string text32 = detail_panel::to32(content);
        int content_cell_len = cell_len(text32);
        // Panel interior = content + 1 space padding each side.
        int interior_width = width > 0 ? (width - 2) : (content_cell_len + 2);
        if (interior_width < content_cell_len + 2) interior_width = content_cell_len + 2;

        std::string top = "\u256D"; // ╭
        std::string bottom = "\u2570"; // ╰
        std::string horiz = "\u2500"; // ─
        for (int i = 0; i < interior_width; ++i) { top += horiz; bottom += horiz; }
        top += "\u256E"; // ╮
        bottom += "\u256F"; // ╯

        std::u32string padded = detail_panel::to32(" ") + text32;
        int pad_right = interior_width - content_cell_len - 1;
        for (int i = 0; i < pad_right; ++i) padded += U' ';

        std::cout << top << "\n";
        std::cout << "\u2502" << detail_panel::to8(padded) << "\u2502\n"; // │ ... │
        std::cout << bottom << "\n";
    }
};

} // namespace rich
