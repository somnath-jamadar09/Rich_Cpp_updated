// Minimal port of rich.console.Console — print() with style + simple markup.
// SCOPE: this is a small real subset, not the full 2698-line console.py
// (no Table/Panel/Live rendering, no width-aware wrapping/justify yet — that
// needs Segment/Text/measure first, on the roadmap). What IS real: styled
// print() emits verified-correct ANSI codes (see style.hpp test), and markup
// parsing handles single-level [style]...[/style] and [style]...[/] tags,
// matching Rich's bbcode-like syntax for the common case.
#pragma once
#include "style.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <stack>

namespace rich {

class Console {
public:
    /// Print plain text with an optional style, e.g. console.print("hi", "bold red").
    void print(const std::string& text, const std::string& style = "") {
        if (style.empty()) {
            std::cout << text << "\n";
        } else {
            std::cout << Style::parse(style).render(text) << "\n";
        }
    }

    /// Print text containing simple markup: "[bold red]hi[/bold red]" or
    /// "[bold red]hi[/]". Supports sequential (non-overlapping-nested-fine)
    /// tags; a matching stack allows real nesting, e.g.
    /// "[bold]a [red]b[/red] c[/bold]".
    void print_markup(const std::string& markup) {
        std::string out;
        std::stack<Style> style_stack;
        style_stack.push(Style{});
        size_t i = 0;
        std::string plain_buffer;

        auto flush = [&]() {
            if (plain_buffer.empty()) return;
            if (style_stack.top().empty())
                out += plain_buffer;
            else
                out += style_stack.top().render(plain_buffer);
            plain_buffer.clear();
        };

        while (i < markup.size()) {
            if (markup[i] == '[') {
                size_t end = markup.find(']', i);
                if (end == std::string::npos) {
                    plain_buffer += markup[i++];
                    continue;
                }
                std::string tag = markup.substr(i + 1, end - i - 1);
                flush();
                if (!tag.empty() && tag[0] == '/') {
                    if (style_stack.size() > 1) style_stack.pop();
                } else {
                    Style combined = style_stack.top() + Style::parse(tag);
                    style_stack.push(combined);
                }
                i = end + 1;
            } else {
                plain_buffer += markup[i++];
            }
        }
        flush();
        std::cout << out << "\n";
    }
};

} // namespace rich
