// Port of rich/_stack.py
#pragma once
#include <vector>

namespace rich {

/// A small shim over std::vector used as a stack.
template <typename T>
class Stack : public std::vector<T> {
public:
    using std::vector<T>::vector;

    /// Get top of stack.
    T& top() { return this->back(); }
    const T& top() const { return this->back(); }

    /// Push an item on to the stack (append in stack nomenclature).
    void push(const T& item) { this->push_back(item); }
    void push(T&& item) { this->push_back(std::move(item)); }
};

} // namespace rich
