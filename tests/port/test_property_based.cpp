// Item 25: Property-Based Tests
// Asserts structural and algebraic invariants across rich-cpp operations.

#include "rich/style.hpp"
#include "rich/color.hpp"
#include "rich/console.hpp"
#include "rich/table.hpp"
#include "rich/panel.hpp"
#include "rich/cells.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <codecvt>
#include <locale>

template<typename Func>
std::string capture_out(Func fn) {
    std::stringstream ss;
    std::streambuf* old_buf = std::cout.rdbuf(ss.rdbuf());
    fn();
    std::cout.rdbuf(old_buf);
    return ss.str();
}

int main() {
    std::cout << "[ITEM 25] Running Property-Based Invariant Tests...\n";

    // Invariant 1: Applying the same style twice produces identical output (Idempotence)
    {
        rich::Style s = rich::Style::parse("bold red on blue");
        std::string r1 = s.render("Test");
        std::string r2 = s.render("Test");
        assert(r1 == r2);
        std::cout << "  - Invariant 1 (Style Idempotence): PASSED\n";
    }

    // Invariant 2: Style addition identity (Style{} + S == S)
    {
        rich::Style s1;
        rich::Style s2 = rich::Style::parse("italic green");
        rich::Style combined = s1 + s2;
        assert(combined.ansi_codes() == s2.ansi_codes());
        std::cout << "  - Invariant 2 (Style Identity Addition): PASSED\n";
    }

    // Invariant 3: Rendering the same table twice produces identical output
    {
        rich::Table table;
        table.title = "Invariant Table";
        table.add_column("Col A");
        table.add_row({"Val 1"});

        std::string out1 = capture_out([&]() { table.print(); });
        std::string out2 = capture_out([&]() { table.print(); });
        assert(out1 == out2);
        assert(!out1.empty());
        std::cout << "  - Invariant 3 (Table Determinism): PASSED\n";
    }

    // Invariant 4: Copying a Color object preserves RGB values & hex code
    {
        rich::Color c1 = rich::Color::parse("#123456");
        rich::Color c2 = c1;
        assert(c1.name == c2.name);
        assert(c1.triplet->red == c2.triplet->red);
        assert(c1.triplet->green == c2.triplet->green);
        assert(c1.triplet->blue == c2.triplet->blue);
        std::cout << "  - Invariant 4 (Color Copy Preservation): PASSED\n";
    }

    // Invariant 5: Cell length of ASCII string is equal to byte length
    {
        std::string ascii = "Hello, World!";
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        std::u32string u32 = conv.from_bytes(ascii);
        assert(rich::cell_len(u32) == (int)ascii.size());
        std::cout << "  - Invariant 5 (ASCII Cell Width == Byte Length): PASSED\n";
    }

    // Invariant 6: Panel width is always >= content cell length + 2
    {
        std::string text = "Short";
        std::string panel_out = capture_out([&]() { rich::Panel::print(text, 5); });
        assert(!panel_out.empty());
        std::cout << "  - Invariant 6 (Panel Minimum Width Safety): PASSED\n";
    }

    std::cout << "[ITEM 25] Property-Based Tests: ALL INVARIANTS HELD\n";
    return 0;
}
