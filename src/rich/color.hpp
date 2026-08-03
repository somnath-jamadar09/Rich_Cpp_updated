// Port of rich/color.py (core subset: ColorType/ColorSystem, Color::parse,
// Color::get_ansi_codes, from_rgb/from_triplet/from_ansi/default).
// NOT ported yet: downgrade() (needs EIGHT_BIT_PALETTE/STANDARD_PALETTE/
// WINDOWS_PALETTE from _palettes.py), get_truecolor() (needs terminal_theme.py).
// Named-color table (235 entries) is mechanically extracted from the real
// Python ANSI_COLOR_NAMES dict, not hand-typed — see _color_names.hpp.
#pragma once
#include "color_triplet.hpp"
#include "_color_names.hpp"
#include <string>
#if __has_include(<optional>)
#include <optional>
#elif __has_include(<experimental/optional>)
#include <experimental/optional>
namespace std { using experimental::optional; using experimental::nullopt; }
#endif
#include <vector>
#include <stdexcept>
#include <regex>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace rich {

enum class ColorSystem { STANDARD = 1, EIGHT_BIT = 2, TRUECOLOR = 3, WINDOWS = 4 };
enum class ColorType { DEFAULT = 0, STANDARD = 1, EIGHT_BIT = 2, TRUECOLOR = 3, WINDOWS = 4 };

class ColorParseError : public std::runtime_error {
public:
    explicit ColorParseError(const std::string& msg) : std::runtime_error(msg) {}
};

/// Terminal color definition. Port of rich.color.Color (a NamedTuple in Python).
struct Color {
    std::string name;
    ColorType type;
    std::optional<int> number;
    std::optional<ColorTriplet> triplet;

    bool is_default() const { return type == ColorType::DEFAULT; }

    static Color default_color() { return Color{"default", ColorType::DEFAULT, {}, {}}; }

    static Color from_ansi(int number) {
        return Color{"color(" + std::to_string(number) + ")",
                     number < 16 ? ColorType::STANDARD : ColorType::EIGHT_BIT, number, {}};
    }

    static Color from_triplet(const ColorTriplet& triplet) {
        return Color{triplet.hex(), ColorType::TRUECOLOR, {}, triplet};
    }

    static Color from_rgb(double red, double green, double blue) {
        return from_triplet(ColorTriplet{(int)red, (int)green, (int)blue});
    }

    /// Parse a color definition: named color, "#rrggbb", "color(N)", or "rgb(r,g,b)".
    static Color parse(const std::string& original_color) {
        std::string color = original_color;
        std::transform(color.begin(), color.end(), color.begin(),
                        [](unsigned char c) { return std::tolower(c); });
        // strip whitespace
        auto not_space = [](unsigned char c) { return !std::isspace(c); };
        color.erase(color.begin(), std::find_if(color.begin(), color.end(), not_space));
        color.erase(std::find_if(color.rbegin(), color.rend(), not_space).base(), color.end());

        if (color == "default") return Color{color, ColorType::DEFAULT, {}, {}};

        const auto& names = color_names::ansi_color_names();
        auto it = names.find(color);
        if (it != names.end()) {
            int n = it->second;
            return Color{color, n < 16 ? ColorType::STANDARD : ColorType::EIGHT_BIT, n, {}};
        }

        static const std::regex re_hex("^#([0-9a-f]{6})$");
        static const std::regex re_8bit("^color\\(([0-9]{1,3})\\)$");
        static const std::regex re_rgb("^rgb\\(([\\d\\s,]+)\\)$");
        std::smatch m;

        if (std::regex_match(color, m, re_hex)) {
            std::string hex = m[1].str();
            ColorTriplet triplet{std::stoi(hex.substr(0, 2), nullptr, 16),
                                  std::stoi(hex.substr(2, 2), nullptr, 16),
                                  std::stoi(hex.substr(4, 2), nullptr, 16)};
            return Color{color, ColorType::TRUECOLOR, {}, triplet};
        }
        if (std::regex_match(color, m, re_8bit)) {
            int number = std::stoi(m[1].str());
            if (number > 255) throw ColorParseError("color number must be <= 255 in '" + color + "'");
            return Color{color, number < 16 ? ColorType::STANDARD : ColorType::EIGHT_BIT, number, {}};
        }
        if (std::regex_match(color, m, re_rgb)) {
            std::string components = m[1].str();
            std::vector<int> parts;
            std::stringstream ss(components);
            std::string tok;
            while (std::getline(ss, tok, ',')) parts.push_back(std::stoi(tok));
            if (parts.size() != 3)
                throw ColorParseError("expected three components in '" + original_color + "'");
            for (int c : parts)
                if (c > 255)
                    throw ColorParseError("color components must be <= 255 in '" + original_color + "'");
            ColorTriplet triplet{parts[0], parts[1], parts[2]};
            return Color{color, ColorType::TRUECOLOR, {}, triplet};
        }

        throw ColorParseError("'" + original_color + "' is not a valid color");
    }

    /// Get the ANSI escape code parts for this color (without ESC[ / m wrapper).
    std::vector<std::string> get_ansi_codes(bool foreground = true) const {
        switch (type) {
            case ColorType::DEFAULT:
                return {foreground ? "39" : "49"};
            case ColorType::WINDOWS:
            case ColorType::STANDARD: {
                int n = *number;
                int fore = n < 8 ? 30 : 82;
                int back = n < 8 ? 40 : 92;
                return {std::to_string((foreground ? fore : back) + n)};
            }
            case ColorType::EIGHT_BIT:
                return {foreground ? "38" : "48", "5", std::to_string(*number)};
            case ColorType::TRUECOLOR: {
                auto& t = *triplet;
                return {foreground ? "38" : "48", "2", std::to_string(t.red),
                        std::to_string(t.green), std::to_string(t.blue)};
            }
        }
        return {};
    }
};

} // namespace rich
