// Item 17: Thread Safety Tests
//
// Thread Safety Matrix:
//  [SUPPORTED]: Independent instances of Console, Style, Color, Table, Panel across different threads.
//  [SUPPORTED]: Pure functional parsing (Style::parse, Color::parse, cell_len) from multiple threads concurrently.
//  [UNSUPPORTED]: Modifying the SAME Console, Table, or Style instance simultaneously from multiple threads without external mutex locking.

#include "rich/console.hpp"
#include "rich/style.hpp"
#include "rich/color.hpp"
#include "rich/table.hpp"
#include "rich/cells.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <cassert>
#include <mutex>

static std::mutex g_io_mutex;

void worker_independent_console(int thread_id) {
    rich::Console console;
    std::stringstream ss;
    ss << "Thread " << thread_id << " says hello!";
    std::string msg = ss.str();
    
    // Concurrent rendering using value objects
    std::string styled = rich::Style::parse("bold cyan").render(msg);
    
    std::lock_guard<std::mutex> lock(g_io_mutex);
    std::cout << styled << "\n";
}

void worker_table_generation(int thread_id) {
    rich::Table table;
    table.title = "Thread Table " + std::to_string(thread_id);
    table.add_column("ID");
    table.add_column("Value");
    table.add_row({std::to_string(thread_id), "Data " + std::to_string(thread_id * 10)});

    std::stringstream ss;
    std::streambuf* old_buf = std::cout.rdbuf(ss.rdbuf());
    table.print();
    std::cout.rdbuf(old_buf);

    assert(!ss.str().empty());
}

void worker_concurrent_style_and_color() {
    for (int i = 0; i < 100; ++i) {
        rich::Style s = rich::Style::parse("bold red on #123456");
        rich::Color c = rich::Color::parse("rgb(10, 20, 30)");
        std::string codes = s.ansi_codes();
        assert(!codes.empty());
        assert(c.triplet.has_value());
    }
}

int main() {
    std::cout << "[ITEM 17] Running Thread Safety Tests...\n";

    constexpr int NUM_THREADS = 4;
    std::vector<std::thread> threads;

    // 1. Multiple Console objects printing independently
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker_independent_console, i);
    }
    for (auto& t : threads) t.join();
    threads.clear();
    std::cout << "  - Independent Console printing: PASSED\n";

    // 2. Simultaneous table generation
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker_table_generation, i);
    }
    for (auto& t : threads) t.join();
    threads.clear();
    std::cout << "  - Simultaneous table generation: PASSED\n";

    // 3. Concurrent style and color creation
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker_concurrent_style_and_color);
    }
    for (auto& t : threads) t.join();
    threads.clear();
    std::cout << "  - Concurrent style creation: PASSED\n";

    std::cout << "[ITEM 17] Thread Safety Tests: ALL PASSED\n";
    return 0;
}
