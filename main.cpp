// Quick smoke test — exercises the ported modules.
#include "rich/cells.hpp"
#include "rich/color_triplet.hpp"
#include "rich/_loop.hpp"
#include "rich/_pick.hpp"
#include "rich/_stack.hpp"
#include "rich/region.hpp"
#include "rich/errors.hpp"
#include "rich/console.hpp"
#include "rich/rule.hpp"
#include "rich/panel.hpp"
#include "rich/table.hpp"

#include <iostream>
#include <codecvt>
#include <locale>

std::u32string to32(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}
std::string to8(const std::u32string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}

int main() {
    std::cout << "cell_len(\"hello\") = " << rich::cell_len(to32("hello")) << "\n";
    std::cout << "cell_len(\"你好\") = " << rich::cell_len(to32("你好")) << "\n";
    std::cout << "cell_len(\"\xF0\x9F\x98\x80\") = " << rich::cell_len(to32("\xF0\x9F\x98\x80")) << "\n";

    rich::ColorTriplet c{255, 0, 128};
    std::cout << "color hex = " << c.hex() << "\n";

    rich::Stack<int> s;
    s.push(1);
    s.push(2);
    std::cout << "stack top = " << s.top() << "\n";

    rich::Console console;
    console.print("Hello, styled world!", "bold red");
    console.print_markup("[bold green]Success[/bold green]: [italic]C++ Rich port is alive[/italic]");
    console.print_markup("[bold]Nested: [red]red text[/red] back to bold[/bold]");
    console.print("Truecolor:", "bold #ff8800 on #001133");

    rich::Rule::print(50, "Rich C++ Port");
    rich::Panel::print("Hi there", 24);
    rich::Panel::print("Panels work too!");
    rich::Rule::print(50);

    rich::Table table;
    table.title = "Star Wars";
    table.add_column("Date");
    table.add_column("Title");
    table.add_row({"Dec 20, 2019", "Rise of Skywalker"});
    table.add_row({"May 25, 2018", "Solo"});
    table.print();

    return 0;
}
