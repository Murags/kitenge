#include <doctest/doctest.h>

#include <cmath>
#include <set>
#include <utility>

#include "core/PatternSettings.h"
#include "motifs/MotifLibrary.h"
#include "render/MotifRenderer.h"
#include "render/PatternRenderer.h"
#include "symmetry/Symmetry.h"

using namespace kitenge;

namespace {

int count(const Framebuffer& fb, Color c) {
    int n = 0;
    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            n += fb.getPixel(x, y) == c ? 1 : 0;
        }
    }
    return n;
}

const std::vector<Color> kPalette = {Color(0, 128, 0), Color(255, 200, 0), Color(200, 80, 50),
                                     Color(30, 30, 30)};

}  // namespace

TEST_CASE("tiles are spaced by tile size plus spacing and centred on the canvas") {
    const auto tiles = layoutTiles(SymmetryRule::Translate, 100, 20, 600, 400);
    REQUIRE_FALSE(tiles.empty());

    bool centreTile = false;
    std::set<std::pair<int, int>> cells;
    for (const TilePlacement& t : tiles) {
        const Vec2 c = t.transform.apply({0, 0});
        // Neighbouring cells are exactly one pitch apart.
        const Vec2 right = t.transform.apply({1, 0}) - t.transform.apply({0, 0});
        CHECK(right.x == doctest::Approx(100.0f));
        CHECK(std::fmod(std::fabs(c.x - 300.0f), 120.0f) == doctest::Approx(0.0f));
        CHECK(std::fmod(std::fabs(c.y - 200.0f), 120.0f) == doctest::Approx(0.0f));
        centreTile =
            centreTile || (c.x == doctest::Approx(300.0f) && c.y == doctest::Approx(200.0f));
        CHECK(cells.insert({t.row, t.col}).second);
    }
    CHECK(centreTile);
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        CHECK(tiles[i].order == static_cast<int>(i));
    }
}

TEST_CASE("the tile grid covers the whole canvas including partial edge tiles") {
    for (int size : {40, 77, 125, 300}) {
        const int w = 853, h = 611, spacing = 9;
        const auto tiles = layoutTiles(SymmetryRule::Translate, size, spacing, w, h);
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        for (const TilePlacement& t : tiles) {
            const Vec2 a = t.transform.apply({-0.5f, -0.5f});
            const Vec2 b = t.transform.apply({0.5f, 0.5f});
            minX = std::min(minX, a.x);
            minY = std::min(minY, a.y);
            maxX = std::max(maxX, b.x);
            maxY = std::max(maxY, b.y);
        }
        CAPTURE(size);
        CHECK(minX <= spacing);
        CHECK(minY <= spacing);
        CHECK(maxX >= w - spacing);
        CHECK(maxY >= h - spacing);
    }
    CHECK(layoutTiles(SymmetryRule::Translate, 0, 0, 100, 100).empty());
    CHECK(layoutTiles(SymmetryRule::Translate, 50, 0, 0, 100).empty());
}

TEST_CASE("drawLine clips to the canvas and drawMotif fills then outlines") {
    Framebuffer fb(50, 50);
    fb.clear(Color(255, 255, 255));
    render::drawLine(fb, {-100, 25}, {200, 25}, kPalette[3]);
    CHECK(count(fb, kPalette[3]) == 50);

    fb.clear(Color(255, 255, 255));
    Motif square{
        "Square",
        {Shape{Polygon{{{-0.4f, -0.4f}, {0.4f, -0.4f}, {0.4f, 0.4f}, {-0.4f, 0.4f}}}, 0, 3}}};
    render::drawMotif(fb, square, Mat3::translate(25, 25) * Mat3::scale(40, 40), kPalette);
    CHECK(fb.getPixel(25, 25) == kPalette[0]);
    CHECK(fb.getPixel(9, 25) == kPalette[3]);
    CHECK(fb.getPixel(2, 2) == Color(255, 255, 255));

    render::RenderOptions noOutline;
    noOutline.outline = false;
    fb.clear(Color(255, 255, 255));
    render::drawMotif(fb, square, Mat3::translate(25, 25) * Mat3::scale(40, 40), kPalette,
                      noOutline);
    CHECK(count(fb, kPalette[3]) == 0);
    CHECK(count(fb, kPalette[0]) == 32 * 32);
}

TEST_CASE("circles stay circles under quarter turns and become polygons otherwise") {
    Motif dot{"Dot", {Shape{Circle{{0, 0}, 0.4f}, 1, kNoColor}}};
    const Mat3 place = Mat3::translate(50, 50) * Mat3::scale(50, 50);
    Framebuffer a(100, 100), b(100, 100), c(100, 100);
    render::drawMotif(a, dot, place, kPalette);
    render::drawMotif(b, dot, place * Mat3::rotate(3.14159265f / 2), kPalette);
    render::drawMotif(c, dot, place * Mat3::rotate(0.5f), kPalette);
    CHECK(count(a, kPalette[1]) == count(b, kPalette[1]));
    const double expected = 3.14159265 * 20 * 20;
    CHECK(std::fabs(count(c, kPalette[1]) - expected) / expected < 0.05);
}

TEST_CASE("every built-in motif draws something inside its tile only") {
    PatternSettings settings;
    for (std::size_t i = 0; i < builtinMotifs().size(); ++i) {
        Framebuffer fb(140, 140);
        fb.clear(settings.background);
        render::drawMotif(fb, builtinMotifs()[i], Mat3::translate(70, 70) * Mat3::scale(100, 100),
                          settings.palette);
        CAPTURE(builtinMotifs()[i].name);
        CHECK(count(fb, settings.background) < 140 * 140 - 500);
        for (int k = 0; k < 140; ++k) {
            CHECK(fb.getPixel(k, 5) == settings.background);
            CHECK(fb.getPixel(5, k) == settings.background);
        }
    }
}

TEST_CASE("drawPattern clears to the background and reports the tiles drawn") {
    PatternSettings settings;
    Framebuffer fb(400, 300);
    const int tiles = render::drawPattern(fb, settings);
    CHECK(tiles ==
          static_cast<int>(
              layoutTiles(settings.rule, settings.tileSize, settings.spacing, 400, 300).size()));
    CHECK(tiles > 0);
    CHECK(count(fb, settings.background) > 0);
    CHECK(count(fb, settings.background) < 400 * 300);
}
