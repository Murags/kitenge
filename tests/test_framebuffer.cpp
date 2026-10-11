#include <doctest/doctest.h>

#include "core/Framebuffer.h"

using kitenge::Color;
using kitenge::Framebuffer;

TEST_CASE("a new framebuffer has the requested size and starts opaque black") {
    Framebuffer fb(4, 3);
    CHECK(fb.width() == 4);
    CHECK(fb.height() == 3);
    CHECK(fb.pitchBytes() == 16);
    CHECK(fb.getPixel(0, 0) == Color(0, 0, 0, 255));
}

TEST_CASE("setPixel writes exactly one pixel") {
    Framebuffer fb(4, 3);
    const Color red(255, 0, 0);
    fb.setPixel(2, 1, red);

    CHECK(fb.getPixel(2, 1) == red);
    CHECK(fb.getPixel(1, 1) != red);
    CHECK(fb.getPixel(2, 0) != red);
    // Row-major layout: (2, 1) is element 1 * width + 2.
    CHECK(fb.data()[1 * 4 + 2] == red);
}

TEST_CASE("writes outside the buffer are ignored") {
    Framebuffer fb(4, 3);
    fb.clear(Color(10, 20, 30));
    fb.setPixel(-1, 0, Color(255, 255, 255));
    fb.setPixel(0, -1, Color(255, 255, 255));
    fb.setPixel(4, 0, Color(255, 255, 255));
    fb.setPixel(0, 3, Color(255, 255, 255));

    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            CHECK(fb.getPixel(x, y) == Color(10, 20, 30));
        }
    }
    CHECK(fb.getPixel(99, 99) == Color(0, 0, 0, 0));
}

TEST_CASE("clear fills every pixel and resize discards contents") {
    Framebuffer fb(2, 2);
    fb.clear(Color::fromHex(0x1F6F57));
    CHECK(fb.getPixel(1, 1) == Color(0x1F, 0x6F, 0x57));

    fb.resize(3, 1);
    CHECK(fb.width() == 3);
    CHECK(fb.height() == 1);
    CHECK(fb.getPixel(0, 0) == Color());

    fb.resize(-5, 2);
    CHECK(fb.width() == 0);
    CHECK_FALSE(fb.contains(0, 0));
}
