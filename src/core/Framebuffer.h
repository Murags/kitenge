#pragma once

#include <vector>

#include "core/Color.h"

namespace kitenge {

// The single surface every algorithm in the project draws into.
//
// Coordinates are integer pixels with the origin at the top-left and y
// pointing down. Pixels are stored row by row with no padding. Writes outside
// the buffer are ignored, so rasterisers can hand over any coordinate and leave
// trimming the visible part to clipping.
class Framebuffer {
public:
    Framebuffer() = default;
    Framebuffer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    // Changes the size and discards the old contents. Negative sizes become 0.
    void resize(int width, int height);

    void clear(Color color);

    bool contains(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }

    void setPixel(int x, int y, Color color);

    // Returns transparent black for coordinates outside the buffer.
    Color getPixel(int x, int y) const;

    // Raw access for presenting the buffer on screen. Row pitch is
    // width() * sizeof(Color) bytes.
    const Color* data() const { return pixels_.data(); }
    int pitchBytes() const { return width_ * static_cast<int>(sizeof(Color)); }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Color> pixels_;
};

}  // namespace kitenge
