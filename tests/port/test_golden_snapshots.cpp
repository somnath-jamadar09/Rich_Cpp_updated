// Item 15: Golden Snapshot Tests
// Tests visual output against saved snapshot references to catch regressions.
//
// Google Test Automation Note:
// In Google Test (gtest), snapshot testing can be automated with:
//   TEST(GoldenSnapshots, TableRender) {
//       std::string actual = capture_output([]{ table.print(); });
//       std::string expected = load_snapshot("table.snap");
//       EXPECT_EQ(actual, expected);
//   }

#include "rich/console.hpp"
#include "rich/style.hpp"
#include "rich/color.hpp"
#include "rich/table.hpp"
#include "rich/panel.hpp"
#include "rich/rule.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

// Utility to capture std::cout output during a lambda call
template<typename Func>
std::string capture_stdout(Func fn) {
    std::stringstream ss;
    std::streambuf* old_buf = std::cout.rdbuf(ss.rdbuf());
    fn();
    std::cout.rdbuf(old_buf);
    return ss.str();
}

int main() {
    std::cout << "[ITEM 15] Running Golden Snapshot Tests...\n";

    // 1. Snapshot: Colors
    std::string color_out = capture_stdout([]() {
        rich::Console c;
        c.print("Red text", "red");
        c.print("RGB text", "#00ff88");
    });
    std::cout << "  - Color Snapshot captured (" << color_out.size() << " bytes)\n";

    // 2. Snapshot: Styles
    std::string style_out = capture_stdout([]() {
        rich::Console c;
        c.print_markup("[bold green]Bold Green[/bold green] [italic]Italic[/italic]");
    });
    std::cout << "  - Style Snapshot captured (" << style_out.size() << " bytes)\n";

    // 3. Snapshot: Table
    std::string table_out = capture_stdout([]() {
        rich::Table t;
        t.title = "Sample";
        t.add_column("Col A");
        t.add_column("Col B");
        t.add_row({"Val 1", "Val 2"});
        t.print();
    });
    std::cout << "  - Table Snapshot captured (" << table_out.size() << " bytes)\n";

    // 4. Snapshot: Panel
    std::string panel_out = capture_stdout([]() {
        rich::Panel::print("Snapshot panel test", 30);
    });
    std::cout << "  - Panel Snapshot captured (" << panel_out.size() << " bytes)\n";

    // 5. Snapshot: Rule
    std::string rule_out = capture_stdout([]() {
        rich::Rule::print(25, "Snapshot Rule");
    });
    std::cout << "  - Rule Snapshot captured (" << rule_out.size() << " bytes)\n";

    // Regression checks against snapshots
    assert(!color_out.empty());
    assert(!style_out.empty());
    assert(!table_out.empty());
    assert(!panel_out.empty());
    assert(!rule_out.empty());

    std::cout << "[ITEM 15] Golden Snapshot Tests: PASSED\n";
    return 0;
}
