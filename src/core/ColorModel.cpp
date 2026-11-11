#include "core/ColorModel.h"

#include <algorithm>
#include <cmath>

namespace kitenge {

namespace {

std::uint8_t toByte(float f) {
    return static_cast<std::uint8_t>(std::clamp(f, 0.0f, 1.0f) * 255.0f + 0.5f);
}

}  // namespace

Hsv toHsv(Color c) {
    const float r = c.r / 255.0f;
    const float g = c.g / 255.0f;
    const float b = c.b / 255.0f;
    const float max = std::max({r, g, b});
    const float min = std::min({r, g, b});
    const float delta = max - min;

    Hsv out;
    out.v = max;
    out.s = max > 0.0f ? delta / max : 0.0f;
    if (delta > 0.0f) {
        // Which sixth of the colour wheel we are in depends on the largest channel.
        if (max == r) {
            out.h = 60.0f * std::fmod((g - b) / delta, 6.0f);
        } else if (max == g) {
            out.h = 60.0f * ((b - r) / delta + 2.0f);
        } else {
            out.h = 60.0f * ((r - g) / delta + 4.0f);
        }
        if (out.h < 0.0f) {
            out.h += 360.0f;
        }
    }
    return out;
}

Color toRgb(Hsv hsv, std::uint8_t alpha) {
    float h = std::fmod(hsv.h, 360.0f);
    if (h < 0.0f) {
        h += 360.0f;
    }
    const float s = std::clamp(hsv.s, 0.0f, 1.0f);
    const float v = std::clamp(hsv.v, 0.0f, 1.0f);

    // Chroma is split between the largest channel (c) and the middle one (x).
    const float c = v * s;
    const float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    const float m = v - c;
    float r = 0, g = 0, b = 0;
    switch (static_cast<int>(h / 60.0f)) {
        case 0:
            r = c, g = x;
            break;
        case 1:
            r = x, g = c;
            break;
        case 2:
            g = c, b = x;
            break;
        case 3:
            g = x, b = c;
            break;
        case 4:
            r = x, b = c;
            break;
        default:
            r = c, b = x;
            break;
    }
    return Color(toByte(r + m), toByte(g + m), toByte(b + m), alpha);
}

}  // namespace kitenge
