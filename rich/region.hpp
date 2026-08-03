// Port of rich/region.py
#pragma once

namespace rich {

/// Defines a rectangular region of the screen.
struct Region {
    int x;
    int y;
    int width;
    int height;
};

} // namespace rich
