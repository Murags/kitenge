#include <doctest/doctest.h>

#include "core/DoubleBuffer.h"
#include "core/PatternSettings.h"
#include "render/PatternRenderer.h"
#include "symmetry/Assembly.h"

using namespace kitenge;

TEST_CASE("the assembly player reveals copies at its rate and then stops") {
    AssemblyPlayer player(10.0f);
    CHECK_FALSE(player.playing());

    player.start(25);
    CHECK(player.playing());
    CHECK(player.visible() == 0);
    CHECK(player.update(0.5f) == 5);
    CHECK(player.update(1.0f) == 15);
    CHECK(player.playing());
    CHECK(player.update(5.0f) == 25);  // never more than the total
    CHECK_FALSE(player.playing());
    CHECK(player.update(1.0f) == 25);  // stays complete once stopped
}

TEST_CASE("an empty pattern or a stopped player does not play") {
    AssemblyPlayer player;
    player.start(0);
    CHECK_FALSE(player.playing());

    player.start(10);
    player.stop();
    CHECK(player.update(10.0f) == 0);
}

TEST_CASE("drawPattern draws only the requested number of tiles") {
    PatternSettings settings;
    Framebuffer fb(400, 300);
    const int total = render::tileCount(settings, fb.width(), fb.height());
    REQUIRE(total > 3);

    CHECK(render::drawPattern(fb, settings, {}, 0) == 0);
    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            REQUIRE(fb.getPixel(x, y) == settings.background);
        }
    }
    CHECK(render::drawPattern(fb, settings, {}, 3) == 3);
    CHECK(render::drawPattern(fb, settings, {}, total + 10) == total);
    CHECK(render::drawPattern(fb, settings) == total);
}

TEST_CASE("the double buffer only shows a frame after swap") {
    DoubleBuffer buffers;
    buffers.resize(4, 4);
    const Color red(255, 0, 0);
    buffers.back().clear(red);
    CHECK(buffers.front().getPixel(0, 0) != red);
    buffers.swap();
    CHECK(buffers.front().getPixel(0, 0) == red);
    CHECK(buffers.back().getPixel(0, 0) != red);
}
