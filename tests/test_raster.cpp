#include <doctest/doctest.h>

#include <cmath>
#include <cstdlib>
#include <vector>

#include "raster/Clip.h"
#include "raster/Raster.h"

using kitenge::Color;
using kitenge::Framebuffer;
using kitenge::Polygon;
using kitenge::Vec2;
namespace raster = kitenge::raster;

namespace {

const Color kInk(255, 0, 0);

struct Pixel {
    int x;
    int y;
};

std::vector<Pixel> inked(const Framebuffer& fb) {
    std::vector<Pixel> out;
    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            if (fb.getPixel(x, y) == kInk) {
                out.push_back({x, y});
            }
        }
    }
    return out;
}

// A line is gap-free when every pixel touches another one (8-connected).
bool connected(const std::vector<Pixel>& pixels) {
    for (const Pixel& p : pixels) {
        bool hasNeighbour = pixels.size() == 1;
        for (const Pixel& q : pixels) {
            const int dx = std::abs(p.x - q.x);
            const int dy = std::abs(p.y - q.y);
            if ((dx | dy) != 0 && dx <= 1 && dy <= 1) {
                hasNeighbour = true;
            }
        }
        if (!hasNeighbour) {
            return false;
        }
    }
    return true;
}

float area(const Polygon& poly) {
    float twice = 0.0f;
    for (std::size_t i = 0; i < poly.points.size(); ++i) {
        const Vec2 a = poly.points[i];
        const Vec2 b = poly.points[(i + 1) % poly.points.size()];
        twice += a.x * b.y - b.x * a.y;
    }
    return std::fabs(twice) / 2.0f;
}

}  // namespace

TEST_CASE("both line algorithms cover every octant with one pixel per major-axis step") {
    const int ends[][2] = {{19, 4},   {4, 19},   {-4, 19}, {-19, 4},
                           {-19, -4}, {-4, -19}, {4, -19}, {19, -4}};
    for (const auto& e : ends) {
        for (int algorithm = 0; algorithm < 2; ++algorithm) {
            Framebuffer fb(41, 41);
            if (algorithm == 0) {
                raster::lineBresenham(fb, 20, 20, 20 + e[0], 20 + e[1], kInk);
            } else {
                raster::lineDDA(fb, 20, 20, 20 + e[0], 20 + e[1], kInk);
            }
            const auto pixels = inked(fb);
            CHECK(pixels.size() == 20u);
            CHECK(fb.getPixel(20, 20) == kInk);
            CHECK(fb.getPixel(20 + e[0], 20 + e[1]) == kInk);
            CHECK(connected(pixels));
        }
    }
}

TEST_CASE("Bresenham and DDA agree on horizontal, vertical and diagonal lines") {
    const int ends[][4] = {{0, 5, 10, 5}, {5, 0, 5, 10}, {0, 0, 10, 10}, {10, 0, 0, 10}};
    for (const auto& e : ends) {
        Framebuffer a(11, 11);
        Framebuffer b(11, 11);
        raster::lineBresenham(a, e[0], e[1], e[2], e[3], kInk);
        raster::lineDDA(b, e[0], e[1], e[2], e[3], kInk);
        for (int y = 0; y < 11; ++y) {
            for (int x = 0; x < 11; ++x) {
                CHECK(a.getPixel(x, y) == b.getPixel(x, y));
            }
        }
    }
}

TEST_CASE("a single-point line draws one pixel") {
    Framebuffer fb(3, 3);
    raster::lineBresenham(fb, 1, 1, 1, 1, kInk);
    CHECK(inked(fb).size() == 1u);
}

TEST_CASE("midpoint circle pixels lie on the circle and are symmetric") {
    const int r = 15;
    Framebuffer fb(41, 41);
    raster::circle(fb, 20, 20, r, kInk);
    const auto pixels = inked(fb);
    REQUIRE_FALSE(pixels.empty());
    for (const Pixel& p : pixels) {
        const double d = std::hypot(p.x - 20, p.y - 20);
        CHECK(std::fabs(d - r) < 1.0);
        CHECK(fb.getPixel(40 - p.x, p.y) == kInk);
        CHECK(fb.getPixel(p.x, 40 - p.y) == kInk);
        CHECK(fb.getPixel(20 + (p.y - 20), 20 + (p.x - 20)) == kInk);
    }
    CHECK(fb.getPixel(20 + r, 20) == kInk);
    CHECK(fb.getPixel(20, 20 - r) == kInk);
    CHECK(connected(pixels));
}

TEST_CASE("a filled circle covers roughly pi r squared pixels") {
    const int r = 30;
    Framebuffer fb(80, 80);
    raster::fillCircle(fb, 40, 40, r, kInk);
    const double expected = 3.14159265 * r * r;
    CHECK(std::fabs(static_cast<double>(inked(fb).size()) - expected) / expected < 0.05);
    CHECK(fb.getPixel(40, 40) == kInk);
    CHECK(fb.getPixel(40 + r + 1, 40) != kInk);
}

TEST_CASE("midpoint ellipse touches its four extreme points and stays near the curve") {
    const int rx = 25;
    const int ry = 10;
    Framebuffer fb(61, 31);
    raster::ellipse(fb, 30, 15, rx, ry, kInk);
    CHECK(fb.getPixel(30 + rx, 15) == kInk);
    CHECK(fb.getPixel(30 - rx, 15) == kInk);
    CHECK(fb.getPixel(30, 15 + ry) == kInk);
    CHECK(fb.getPixel(30, 15 - ry) == kInk);
    const auto pixels = inked(fb);
    for (const Pixel& p : pixels) {
        const double u = (p.x - 30) / static_cast<double>(rx);
        const double v = (p.y - 15) / static_cast<double>(ry);
        CHECK(std::fabs(std::sqrt(u * u + v * v) - 1.0) < 0.12);
    }
    CHECK(connected(pixels));
}

TEST_CASE("a filled ellipse covers roughly pi rx ry pixels") {
    Framebuffer fb(100, 60);
    raster::fillEllipse(fb, 50, 30, 40, 20, kInk);
    const double expected = 3.14159265 * 40 * 20;
    CHECK(std::fabs(static_cast<double>(inked(fb).size()) - expected) / expected < 0.05);
}

TEST_CASE("Cohen-Sutherland accepts, rejects and trims lines") {
    int x0 = 2, y0 = 2, x1 = 8, y1 = 8;
    CHECK(raster::clipLine(x0, y0, x1, y1, 0, 0, 10, 10));
    CHECK((x0 == 2 && y0 == 2 && x1 == 8 && y1 == 8));

    x0 = -5, y0 = -5, x1 = -1, y1 = 20;
    CHECK_FALSE(raster::clipLine(x0, y0, x1, y1, 0, 0, 10, 10));

    x0 = -10, y0 = 5, x1 = 20, y1 = 5;
    CHECK(raster::clipLine(x0, y0, x1, y1, 0, 0, 10, 10));
    CHECK((x0 == 0 && y0 == 5 && x1 == 10 && y1 == 5));

    x0 = -5, y0 = -5, x1 = 15, y1 = 15;
    CHECK(raster::clipLine(x0, y0, x1, y1, 0, 0, 10, 10));
    CHECK((x0 == 0 && y0 == 0 && x1 == 10 && y1 == 10));

    // Crosses two corners' outside regions but misses the window.
    x0 = -5, y0 = 8, x1 = 3, y1 = 16;
    CHECK_FALSE(raster::clipLine(x0, y0, x1, y1, 0, 0, 10, 10));
}

TEST_CASE("Sutherland-Hodgman keeps inside polygons and trims overlapping ones") {
    const raster::ClipRect window{0, 0, 10, 10};

    const Polygon inside{{{2, 2}, {8, 2}, {8, 8}, {2, 8}}};
    CHECK(raster::clipPolygon(inside, window).points.size() == 4u);

    const Polygon outside{{{20, 20}, {30, 20}, {30, 30}}};
    CHECK(raster::clipPolygon(outside, window).points.empty());

    // A 10x10 square shifted half out on the right and bottom keeps a 5x5 corner.
    const Polygon shifted{{{5, 5}, {15, 5}, {15, 15}, {5, 15}}};
    const Polygon clipped = raster::clipPolygon(shifted, window);
    CHECK(area(clipped) == doctest::Approx(25.0f));
    for (const Vec2& p : clipped.points) {
        CHECK(p.x <= 10.0f);
        CHECK(p.y <= 10.0f);
    }

    // A triangle poking out of the left edge gains a vertex when cut.
    const Polygon triangle{{{-5, 5}, {5, 0}, {5, 10}}};
    CHECK(raster::clipPolygon(triangle, window).points.size() == 4u);
}
