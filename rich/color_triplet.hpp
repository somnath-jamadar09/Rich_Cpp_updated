// Port of rich/color_triplet.py
#pragma once
#include <cstdint>
#include <tuple>
#include <string>
#include <cstdio>

namespace rich {

/// The red, green, and blue components of a color.
struct ColorTriplet {
    int red;   ///< Red component in 0 to 255 range.
    int green; ///< Green component in 0 to 255 range.
    int blue;  ///< Blue component in 0 to 255 range.

    /// Get the color triplet in CSS style, e.g. "#ff0000".
    std::string hex() const {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", red, green, blue);
        return std::string(buf);
    }

    /// The color in RGB format, e.g. "rgb(100,23,255)".
    std::string rgb() const {
        return "rgb(" + std::to_string(red) + "," + std::to_string(green) + "," +
               std::to_string(blue) + ")";
    }

    /// Convert components into floats between 0 and 1.
    std::tuple<double, double, double> normalized() const {
        return {red / 255.0, green / 255.0, blue / 255.0};
    }

    bool operator==(const ColorTriplet& other) const {
        return red == other.red && green == other.green && blue == other.blue;
    }
};

} // namespace rich
