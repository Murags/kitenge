#include <doctest/doctest.h>

#include <cmath>

#include "geometry/Bezier.h"
#include "geometry/Fill.h"
#include "geometry/PolygonOps.h"
#include "geometry/Spline.h"

using kitenge::Color;
using kitenge::Framebuffer;
using kitenge::Polygon;
using kitenge::Vec2;
namespace fill = kitenge::fill;
namespace polygon = kitenge::polygon;

namespace {

const Color kPaper(255, 255, 255);
const Color kInk(0, 0, 0);
const Color kPaint(200, 30, 30);

int count(const Framebuffer& fb, Color c) {
    int n = 0;
    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            n += fb.getPixel(x, y) == c ? 1 : 0;
        }
    }
    return n;
}

void checkNear(Vec2 actual, Vec2 expected, float eps = 1e-4f) {
    CHECK(actual.x == doctest::Approx(expected.x).epsilon(eps));
    CHECK(actual.y == doctest::Approx(expected.y).epsilon(eps));
}

}  // namespace

TEST_CASE("scan-line fill of an axis-aligned rectangle fills exactly its pixels") {
    Framebuffer fb(20, 20);
    fb.clear(kPaper);
    fill::polygon(fb, Polygon{{{2, 3}, {12, 3}, {12, 9}, {2, 9}}}, kPaint);
    CHECK(count(fb, kPaint) == 10 * 6);
    CHECK(fb.getPixel(2, 3) == kPaint);
    CHECK(fb.getPixel(11, 8) == kPaint);
    CHECK(fb.getPixel(12, 8) == kPaper);
    CHECK(fb.getPixel(11, 9) == kPaper);
}

TEST_CASE("scan-line fill matches the polygon area and the even-odd rule") {
    Framebuffer fb(200, 200);
    fb.clear(kPaper);
    const Polygon tri{{{10, 10}, {190, 40}, {60, 180}}};
    fill::polygon(fb, tri, kPaint);
    const float a = polygon::area(tri);
    CHECK(std::fabs(count(fb, kPaint) - a) / a < 0.02f);

    // A five-pointed star drawn as one self-crossing outline leaves the
    // centre pentagon empty under the even-odd rule.
    fb.clear(kPaper);
    Polygon pentagram;
    for (int i = 0; i < 5; ++i) {
        const float t = -1.5707963f + 2.5132741f * static_cast<float>(i);
        pentagram.points.push_back({100 + 90 * std::cos(t), 100 + 90 * std::sin(t)});
    }
    fill::polygon(fb, pentagram, kPaint);
    CHECK(fb.getPixel(100, 100) == kPaper);
    CHECK(fb.getPixel(100, 25) == kPaint);
}

TEST_CASE("adjacent polygons sharing an edge neither overlap nor leave a gap") {
    Framebuffer fb(20, 20);
    fb.clear(kPaper);
    fill::polygon(fb, Polygon{{{0, 0}, {20, 0}, {0, 20}}}, kPaint);
    fill::polygon(fb, Polygon{{{20, 0}, {20, 20}, {0, 20}}}, kInk);
    CHECK(count(fb, kPaint) + count(fb, kInk) == 400);
    CHECK(count(fb, kPaper) == 0);
}

TEST_CASE("scan-line fill clips to the framebuffer") {
    Framebuffer fb(10, 10);
    fb.clear(kPaper);
    fill::polygon(fb, Polygon{{{-50, -50}, {60, -50}, {60, 60}, {-50, 60}}}, kPaint);
    CHECK(count(fb, kPaint) == 100);
}

TEST_CASE("boundary fill stops at the outline, flood fill replaces one region") {
    Framebuffer fb(12, 12);
    fb.clear(kPaper);
    for (int i = 2; i <= 9; ++i) {
        fb.setPixel(i, 2, kInk);
        fb.setPixel(i, 9, kInk);
        fb.setPixel(2, i, kInk);
        fb.setPixel(9, i, kInk);
    }
    fill::boundaryFill(fb, 5, 5, kPaint, kInk);
    CHECK(count(fb, kPaint) == 6 * 6);
    CHECK(fb.getPixel(0, 0) == kPaper);

    fill::floodFill(fb, 0, 0, kInk);
    CHECK(count(fb, kPaper) == 0);
    CHECK(count(fb, kPaint) == 36);
}

TEST_CASE("polygon area, centroid, containment and convexity") {
    const Polygon square{{{0, 0}, {4, 0}, {4, 4}, {0, 4}}};
    CHECK(polygon::area(square) == doctest::Approx(16.0f));
    CHECK(polygon::signedArea(square) > 0.0f);  // clockwise on screen
    checkNear(polygon::centroid(square), {2, 2});
    CHECK(polygon::contains(square, {1, 1}));
    CHECK_FALSE(polygon::contains(square, {5, 1}));
    CHECK(polygon::isConvex(square));

    const Polygon arrow{{{0, 0}, {4, 2}, {0, 4}, {1, 2}}};
    CHECK_FALSE(polygon::isConvex(arrow));
    CHECK_FALSE(polygon::contains(arrow, {0.5f, 2.0f}));

    const Polygon hex = polygon::regular({0, 0}, 1.0f, 6);
    CHECK(hex.points.size() == 6u);
    CHECK(polygon::area(hex) == doctest::Approx(3.0f * std::sqrt(3.0f) / 2.0f));
    CHECK(polygon::star({0, 0}, 2.0f, 1.0f, 5).points.size() == 10u);
    CHECK_FALSE(polygon::isConvex(polygon::star({0, 0}, 2.0f, 1.0f, 5)));
}

TEST_CASE("general de Casteljau matches the cubic form and flatten closes the path") {
    const Vec2 p0{0, 0}, p1{0, 1}, p2{1, 1}, p3{1, 0};
    for (float t : {0.0f, 0.3f, 0.5f, 0.8f, 1.0f}) {
        checkNear(kitenge::bezier::point({p0, p1, p2, p3}, t),
                  kitenge::bezier::point(p0, p1, p2, p3, t));
    }
    // A quadratic peaks at half the height of its middle control point.
    checkNear(kitenge::bezier::point({{0, 0}, {1, 2}, {2, 0}}, 0.5f), {1, 1});
    CHECK(kitenge::bezier::sample({p0, p1, p2, p3}, 10).size() == 11u);

    kitenge::BezierPath lens{
        {0, 0}, {{{0.3f, -0.4f}, {0.7f, -0.4f}, {1, 0}}, {{0.7f, 0.4f}, {0.3f, 0.4f}, {0, 0}}}};
    const Polygon poly = kitenge::bezier::flatten(lens, 8);
    CHECK(poly.points.size() == 16u);
    CHECK(polygon::contains(poly, {0.5f, 0.0f}));
}

TEST_CASE("Catmull-Rom passes through its points; the B-spline stays inside them") {
    const std::vector<Vec2> pts{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    const auto cr = kitenge::spline::catmullRom(pts, 8, true);
    CHECK(cr.size() == 32u);
    for (std::size_t i = 0; i < pts.size(); ++i) {
        checkNear(cr[i * 8], pts[i]);
    }

    const auto open = kitenge::spline::catmullRom(pts, 8, false);
    CHECK(open.size() == 25u);
    checkNear(open.front(), pts.front());
    checkNear(open.back(), pts.back());

    const auto bs = kitenge::spline::bSpline(pts, 8, true);
    CHECK(bs.size() == 32u);
    const Polygon hull{pts};
    for (Vec2 p : bs) {
        CHECK(polygon::contains(hull, p));
    }
    // A uniform B-spline at a knot is (p0 + 4 p1 + p2) / 6, not p1 itself.
    checkNear(bs[8], (pts[0] + pts[1] * 4.0f + pts[2]) / 6.0f);
}
