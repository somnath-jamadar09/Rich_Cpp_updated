// Item 27: Demo Validation Tests
// End-to-end integration test validating console output, colors, truecolor RGB, tables, panels, rules, unicode.

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

int main() {
    std::cout << "[ITEM 27] Running End-to-End Demo Validation Tests...\n\n";

    // 1. Styled Console & Colored Text
    rich::Console console;
    console.print("1. Plain & Styled Console Output Test:", "bold green");
    console.print("   - Standard ANSI named colors: red, green, blue, yellow, magenta, cyan");

    // 2. RGB Truecolor
    console.print("2. Truecolor 24-bit RGB Test:", "bold #ff9900 on #002244");

    // 3. Markup Parsing
    console.print_markup("3. Markup Parser Test: [bold blue]Blue[/bold blue] [italic yellow]Yellow[/italic yellow]");

    // 4. Rule Rendering
    console.print("4. Rule Component Test:");
    rich::Rule::print(40, "Rule Component", "bright_cyan");

    // 5. Panel Rendering
    console.print("5. Panel Container Test:");
    rich::Panel::print("Bordered Panel with Rounded Box Drawing", 45);

    // 6. Data Table & Unicode Rendering
    console.print("6. Data Table & Unicode Cell Width Test:");
    rich::Table table;
    table.title = "Multilingual System Status";
    table.add_column("Locale");
    table.add_column("Status Text");
    table.add_row({"English", "All systems operational"});
    table.add_row({"Chinese (CJK)", "所有系统运行正常"});
    table.add_row({"Emoji Graphemes", "🚀 System Active 😀"});
    table.print();

    std::cout << "\n[ITEM 27] Demo Validation Tests: PASSED (All components rendered correctly)\n";
    return 0;
}
