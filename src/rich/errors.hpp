// Port of rich/errors.py
#pragma once
#include <stdexcept>
#include <string>

namespace rich {

/// An error in console operation.
class ConsoleError : public std::runtime_error {
public:
    explicit ConsoleError(const std::string& msg = "An error in console operation.")
        : std::runtime_error(msg) {}
};

/// An error in styles.
class StyleError : public std::runtime_error {
public:
    explicit StyleError(const std::string& msg = "An error in styles.")
        : std::runtime_error(msg) {}
};

/// Style was badly formatted.
class StyleSyntaxError : public ConsoleError {
public:
    explicit StyleSyntaxError(const std::string& msg = "Style was badly formatted.")
        : ConsoleError(msg) {}
};

/// No such style.
class MissingStyle : public StyleError {
public:
    explicit MissingStyle(const std::string& msg = "No such style.")
        : StyleError(msg) {}
};

/// Style stack is invalid.
class StyleStackError : public ConsoleError {
public:
    explicit StyleStackError(const std::string& msg = "Style stack is invalid.")
        : ConsoleError(msg) {}
};

/// Object is not renderable.
class NotRenderableError : public ConsoleError {
public:
    explicit NotRenderableError(const std::string& msg = "Object is not renderable.")
        : ConsoleError(msg) {}
};

/// Markup was badly formatted.
class MarkupError : public ConsoleError {
public:
    explicit MarkupError(const std::string& msg = "Markup was badly formatted.")
        : ConsoleError(msg) {}
};

/// Error related to Live display.
class LiveError : public ConsoleError {
public:
    explicit LiveError(const std::string& msg = "Error related to Live display.")
        : ConsoleError(msg) {}
};

/// Alt screen mode was required.
class NoAltScreen : public ConsoleError {
public:
    explicit NoAltScreen(const std::string& msg = "Alt screen mode was required.")
        : ConsoleError(msg) {}
};

} // namespace rich
