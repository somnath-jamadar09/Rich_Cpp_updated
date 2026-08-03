// Item 16: Memory Safety Tests
// Verifies correct object lifetime, value semantics, RAII stack unwinding,
// vector buffer reallocation safety, and absence of dangling references or double deletions.
//
// Recommended Compiler Sanitizer Commands & Expected Results:
//   1. AddressSanitizer (ASan):
//      g++ -std=c++17 -fsanitize=address -g -I. tests/test_memory_safety.cpp -o test_asan
//      Expected Output: 0 errors, 0 memory leaks, 0 heap-use-after-free, 0 stack-buffer-overflow.
//   2. LeakSanitizer (LSan):
//      g++ -std=c++17 -fsanitize=leak -g -I. tests/test_memory_safety.cpp -o test_lsan
//      Expected Output: "SUMMARY: LeakSanitizer: 0 byte(s) leaked in 0 allocation(s)."
//   3. UndefinedBehaviorSanitizer (UBSan):
//      g++ -std=c++17 -fsanitize=undefined -g -I. tests/test_memory_safety.cpp -o test_ubsan
//      Expected Output: Clean execution without runtime UB reports.

#include "rich/_stack.hpp"
#include "rich/color.hpp"
#include "rich/style.hpp"
#include "rich/table.hpp"
#include "rich/panel.hpp"
#include "rich/console.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>

void test_stack_lifetime() {
    rich::Stack<std::string> stack;
    stack.push("item1");
    stack.push("item2");
    assert(stack.top() == "item2");
    stack.pop();
    assert(stack.top() == "item1");
    stack.pop();
    assert(stack.empty());
}

void test_color_and_style_value_semantics() {
    // Value objects copied and assigned without pointer invalidation
    rich::Color c1 = rich::Color::parse("red");
    rich::Color c2 = c1; // copy
    assert(c2.name == c1.name);

    rich::Style s1 = rich::Style::parse("bold green");
    rich::Style s2 = s1; // copy
    rich::Style s3 = std::move(s1); // move
    assert(!s2.empty());
    assert(!s3.empty());
}

void test_table_dynamic_reallocation() {
    // Table manages strings inside std::vector, avoiding dangling pointers on expand
    rich::Table table;
    table.title = "Large Table";
    for (int i = 0; i < 100; ++i) {
        table.add_column("Col_" + std::to_string(i));
    }
    for (int r = 0; r < 50; ++r) {
        table.add_row({"Cell_" + std::to_string(r), "Data"});
    }
    assert(table.columns.size() == 100);
    assert(table.rows.size() == 50);
}

void test_exception_safety() {
    // Ensure thrown exceptions unwinding stack clean up all transient objects safely
    try {
        rich::Color::parse("invalid_color_name_xyz");
        assert(false); // should not reach
    } catch (const rich::ColorParseError& e) {
        // Handled cleanly
    }

    try {
        rich::Style::parse("on"); // invalid: missing color after on
        assert(false);
    } catch (const rich::StyleSyntaxError& e) {
        // Handled cleanly
    }
}

int main() {
    std::cout << "[ITEM 16] Running Memory Safety Tests...\n";

    test_stack_lifetime();
    std::cout << "  - Stack lifetime & pop safety: PASSED\n";

    test_color_and_style_value_semantics();
    std::cout << "  - Value semantics & copy/move safety: PASSED\n";

    test_table_dynamic_reallocation();
    std::cout << "  - Vector reallocation & memory bounds: PASSED\n";

    test_exception_safety();
    std::cout << "  - Exception safety & unwinding: PASSED\n";

    std::cout << "[ITEM 16] Memory Safety Tests: ALL PASSED\n";
    return 0;
}
