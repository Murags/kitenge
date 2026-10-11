#include "core/Framebuffer.h"

#include <algorithm>
#include <cstddef>

namespace kitenge {

Framebuffer::Framebuffer(int width, int height) {
    resize(width, height);
}

void Framebuffer::resize(int width, int height) {
    width_ = std::max(width, 0);
    height_ = std::max(height, 0);
    pixels_.assign(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_), Color());
}

void Framebuffer::clear(Color color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void Framebuffer::setPixel(int x, int y, Color color) {
    if (!contains(x, y)) {
        return;
    }
    pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
            static_cast<std::size_t>(x)] = color;
}

Color Framebuffer::getPixel(int x, int y) const {
    if (!contains(x, y)) {
        return Color(0, 0, 0, 0);
    }
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
                   static_cast<std::size_t>(x)];
}

}  // namespace kitenge
