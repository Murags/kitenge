#pragma once

#include <cstdint>

#include "core/Color.h"

namespace kitenge {

// A colour in the HSV model: hue as an angle round the colour wheel, and
// saturation and value from 0 to 1. Easier to reason about when picking
// colours than RGB: change the hue and keep the same strength and brightness.
struct Hsv {
    float h = 0.0f;  // degrees, [0, 360)
    float s = 0.0f;  // [0, 1]
    float v = 0.0f;  // [0, 1]
};

// RGB to HSV. Greys (no saturation) get hue 0.
Hsv toHsv(Color c);

// HSV to RGB. Out-of-range values are wrapped (hue) or clamped (s and v).
Color toRgb(Hsv hsv, std::uint8_t alpha = 255);

}  // namespace kitenge
