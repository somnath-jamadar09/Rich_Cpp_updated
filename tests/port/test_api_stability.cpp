// Item 18: API Stability Tests
// Validates class contracts, constructors, copy/move operations, const correctness, and namespace scoping.

#include "rich/color.hpp"
#include "rich/style.hpp"
#include "rich/console.hpp"
#include "rich/table.hpp"
#include "rich/panel.hpp"
#include "rich/rule.hpp"
#include "rich/region.hpp"
#include "rich/color_triplet.hpp"
#include "rich/_stack.hpp"
#include <iostream>
#include <type_traits>
#include <cassert>

void test_constructors_and_copy_move() {
    // ColorTriplet
    rich::ColorTriplet ct1{10, 20, 30};
    rich::ColorTriplet ct2 = ct1; // copy
    rich::ColorTriplet ct3 = std::move(ct1); // move
    assert(ct2.red == 10 && ct2.green == 20 && ct2.blue == 30);
    assert(ct3.red == 10 && ct3.green == 20 && ct3.blue == 30);

    // Color
    rich::Color c1 = rich::Color::parse("red");
    rich::Color c2(c1); // copy construct
    rich::Color c3(std::move(c1)); // move construct
    rich::Color c4 = c2; // copy assign
    rich::Color c5 = std::move(c2); // move assign
    assert(c4.name == "red");

    // Style
    rich::Style s1 = rich::Style::parse("bold red");
    rich::Style s2(s1);
    rich::Style s3(std::move(s1));
    rich::Style s4 = s2;
    rich::Style s5 = std::move(s2);
    assert(!s4.empty());

    // Table
    rich::Table t1;
    t1.title = "Test";
    t1.add_column("Col1");
    rich::Table t2(t1); // copy
    rich::Table t3(std::move(t1)); // move
    assert(t2.columns.size() == 1);
    assert(t3.columns.size() == 1);
}

void test_const_correctness() {
    const rich::Style const_style = rich::Style::parse("bold blue");
    std::string ansi = const_style.ansi_codes();
    std::string rendered = const_style.render("text");
    bool empty = const_style.empty();
    assert(!ansi.empty());
    assert(!rendered.empty());
    assert(!empty);

    const rich::Table const_table;
    assert(const_table.columns.empty());
    assert(const_table.rows.empty());
}

void test_type_traits() {
    static_assert(std::is_copy_constructible_v<rich::ColorTriplet>, "ColorTriplet must be copy constructible");
    static_assert(std::is_move_constructible_v<rich::ColorTriplet>, "ColorTriplet must be move constructible");
    static_assert(std::is_copy_constructible_v<rich::Color>, "Color must be copy constructible");
    static_assert(std::is_move_constructible_v<rich::Color>, "Color must be move constructible");
    static_assert(std::is_copy_constructible_v<rich::Style>, "Style must be copy constructible");
    static_assert(std::is_move_constructible_v<rich::Style>, "Style must be move constructible");
    static_assert(std::is_copy_constructible_v<rich::Table>, "Table must be copy constructible");
    static_assert(std::is_move_constructible_v<rich::Table>, "Table must be move constructible");
}

int main() {
    std::cout << "[ITEM 18] Running API Stability Tests...\n";

    test_constructors_and_copy_move();
    std::cout << "  - Copy & Move semantics: PASSED\n";

    test_const_correctness();
    std::cout << "  - Const correctness: PASSED\n";

    test_type_traits();
    std::cout << "  - Type trait assertions: PASSED\n";

    std::cout << "[ITEM 18] API Stability Tests: ALL PASSED\n";
    return 0;
}
