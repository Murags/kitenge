#pragma once

#include <cstdint>

namespace kitenge {

// An 8-bit-per-channel RGBA colour. The memory layout (r, g, b, a) matches
// SDL_PIXELFORMAT_RGBA32, so a buffer of Colors can be shown on screen as-is.
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    constexpr Color() = default;
    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    // Builds a colour from 0xRRGGBB, e.g. Color::fromHex(0x1F7A5C).
    static constexpr Color fromHex(std::uint32_t rgb) {
        return Color(static_cast<std::uint8_t>(rgb >> 16), static_cast<std::uint8_t>(rgb >> 8),
                     static_cast<std::uint8_t>(rgb));
    }

    constexpr bool operator==(const Color& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    constexpr bool operator!=(const Color& o) const { return !(*this == o); }
};

static_assert(sizeof(Color) == 4, "Color must stay tightly packed to match RGBA32");

}  // namespace kitenge
