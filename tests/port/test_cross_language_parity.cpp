// Item 14: Cross-Language Port Consistency Tests (C++ side)
// Outputs exact ANSI sequences, layout rendering, RGB color codes, style nesting,
// Unicode cell calculations, and border lines for byte-for-byte comparison with Python Rich.

#include "rich/console.hpp"
#include "rich/style.hpp"
#include "rich/color.hpp"
#include "rich/table.hpp"
#include "rich/panel.hpp"
#include "rich/rule.hpp"
#include "rich/cells.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <codecvt>
#include <locale>

static std::u32string to_u32(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}

int main() {
    std::cout << "--- BEGIN CROSS-LANGUAGE TEST OUTPUT ---\n";

    // 1. ANSI escape sequences
    rich::Style s1 = rich::Style::parse("bold red on blue");
    std::cout << "ANSI_S1: " << s1.ansi_codes() << "\n";
    std::cout << "RENDER_S1: " << s1.render("Hello") << "\n";

    // 2. RGB color encoding
    rich::Color c_rgb = rich::Color::parse("#ff007f");
    std::cout << "RGB_HEX: " << c_rgb.name << "\n";
    std::vector<std::string> codes = c_rgb.get_ansi_codes(true);
    std::cout << "RGB_FORE: ";
    for (size_t i = 0; i < codes.size(); ++i) {
        if (i) std::cout << ";";
        std::cout << codes[i];
    }
    std::cout << "\n";

    // 3. Style nesting
    rich::Style parent = rich::Style::parse("bold green");
    rich::Style child = rich::Style::parse("italic red");
    rich::Style combined = parent + child;
    std::cout << "NESTED_ANSI: " << combined.ansi_codes() << "\n";

    // 4. Unicode width rendering
    std::string text_ascii = "hello";
    std::string text_cjk = "你好世界";
    std::string text_emoji = "😀😃😄";
    std::cout << "WIDTH_ASCII: " << rich::cell_len(to_u32(text_ascii)) << "\n";
    std::cout << "WIDTH_CJK: " << rich::cell_len(to_u32(text_cjk)) << "\n";
    std::cout << "WIDTH_EMOJI: " << rich::cell_len(to_u32(text_emoji)) << "\n";

    // 5. Rule dashes
    std::cout << "--- RULE DEMO ---\n";
    rich::Rule::print(30, "Test");

    // 6. Panel border rendering
    std::cout << "--- PANEL DEMO ---\n";
    rich::Panel::print("Hi there", 20);

    // 7. Table rendering
    std::cout << "--- TABLE DEMO ---\n";
    rich::Table table;
    table.title = "Star Wars";
    table.add_column("Date");
    table.add_column("Title");
    table.add_row({"Dec 20, 2019", "Rise of Skywalker"});
    table.add_row({"May 25, 2018", "Solo"});
    table.print();

    std::cout << "--- END CROSS-LANGUAGE TEST OUTPUT ---\n";
    return 0;
}
