#include <doctest/doctest.h>

#include <cstdlib>

#include "core/ColorModel.h"

using namespace kitenge;

namespace {

void checkHsv(Color c, float h, float s, float v) {
    const Hsv out = toHsv(c);
    CHECK(out.h == doctest::Approx(h).epsilon(1e-3));
    CHECK(out.s == doctest::Approx(s).epsilon(1e-3));
    CHECK(out.v == doctest::Approx(v).epsilon(1e-3));
}

}  // namespace

TEST_CASE("primary and secondary colours sit at multiples of 60 degrees") {
    checkHsv(Color(255, 0, 0), 0, 1, 1);
    checkHsv(Color(255, 255, 0), 60, 1, 1);
    checkHsv(Color(0, 255, 0), 120, 1, 1);
    checkHsv(Color(0, 255, 255), 180, 1, 1);
    checkHsv(Color(0, 0, 255), 240, 1, 1);
    checkHsv(Color(255, 0, 255), 300, 1, 1);
}

TEST_CASE("greys have no saturation and black has no value") {
    checkHsv(Color(0, 0, 0), 0, 0, 0);
    checkHsv(Color(255, 255, 255), 0, 0, 1);
    checkHsv(Color(128, 128, 128), 0, 0, 128 / 255.0f);
}

TEST_CASE("HSV to RGB matches known colours and keeps alpha") {
    CHECK(toRgb({0, 1, 1}) == Color(255, 0, 0));
    CHECK(toRgb({120, 1, 1}) == Color(0, 255, 0));
    CHECK(toRgb({240, 1, 0.5f}) == Color(0, 0, 128));
    CHECK(toRgb({30, 0, 1}) == Color(255, 255, 255));
    CHECK(toRgb({0, 1, 1}, 40).a == 40);
}

TEST_CASE("hue wraps and saturation and value are clamped") {
    CHECK(toRgb({360, 1, 1}) == toRgb({0, 1, 1}));
    CHECK(toRgb({-120, 1, 1}) == toRgb({240, 1, 1}));
    CHECK(toRgb({0, 2, 5}) == toRgb({0, 1, 1}));
}

TEST_CASE("RGB to HSV and back gives the same colour") {
    for (int r = 0; r < 256; r += 15) {
        for (int g = 0; g < 256; g += 15) {
            for (int b = 0; b < 256; b += 15) {
                const Color c(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g),
                              static_cast<std::uint8_t>(b));
                const Color back = toRgb(toHsv(c));
                CAPTURE(r);
                CAPTURE(g);
                CAPTURE(b);
                CHECK(std::abs(back.r - c.r) <= 1);
                CHECK(std::abs(back.g - c.g) <= 1);
                CHECK(std::abs(back.b - c.b) <= 1);
            }
        }
    }
}
