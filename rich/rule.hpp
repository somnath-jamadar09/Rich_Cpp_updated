// Port of rich/rule.py (core subset: default centered rule, "─" characters,
// default style "bright_green" i.e. ANSI 92). Left/right dash-count formula
// verified against real Python: for width=40, title="Title" ->
// left=16, right=17 dashes (chars = width - len(title) - 2; left = chars/2).
// NOT ported: custom characters, align="left"/"right", Console width
// auto-detection (caller must supply width explicitly here).
#pragma once
#include "style.hpp"
#include <string>
#include <iostream>

namespace rich {

class Rule {
public:
    std::string title;
    std::string style = "bright_green"; // rich's default "rule.line" style
    char character = '\xE2'; // placeholder; we emit the UTF-8 box char directly below

    static void print(int width, const std::string& title = "",
                       const std::string& style = "bright_green") {
        const std::string dash = "\u2500"; // "─"
        if (title.empty()) {
            std::string line;
            for (int i = 0; i < width; ++i) line += dash;
            std::cout << Style::parse(style).render(line) << "\n";
            return;
        }
        int title_len = (int)title.size(); // NOTE: byte length; fine for ASCII titles
        int chars = width - title_len - 2;
        if (chars < 2) {
            // Not enough room for a rule; just print the title.
            std::cout << title << "\n";
            return;
        }
        int left = chars / 2;
        int right = chars - left;
        std::string left_dashes, right_dashes;
        for (int i = 0; i < left; ++i) left_dashes += dash;
        for (int i = 0; i < right; ++i) right_dashes += dash;

        Style s = Style::parse(style);
        std::cout << s.render(left_dashes + " ") << title << s.render(" " + right_dashes) << "\n";
    }
};

} // namespace rich
