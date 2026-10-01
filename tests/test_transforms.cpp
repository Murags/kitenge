#include <doctest/doctest.h>

#include "geometry/Bezier.h"
#include "symmetry/Mat3.h"

using kitenge::Mat3;
using kitenge::Vec2;

namespace {

constexpr float kPi = 3.14159265358979f;

void checkNear(Vec2 actual, Vec2 expected) {
    CHECK(actual.x == doctest::Approx(expected.x).epsilon(1e-5));
    CHECK(actual.y == doctest::Approx(expected.y).epsilon(1e-5));
}

}  // namespace

TEST_CASE("basic transforms move a point as expected") {
    checkNear(Mat3::identity().apply({3, 4}), {3, 4});
    checkNear(Mat3::translate(10, -2).apply({3, 4}), {13, 2});
    checkNear(Mat3::scale(2, 3).apply({3, 4}), {6, 12});
    checkNear(Mat3::mirrorX().apply({3, 4}), {-3, 4});
    checkNear(Mat3::mirrorY().apply({3, 4}), {3, -4});
}

TEST_CASE("rotating by 90 degrees four times returns to the start") {
    const Mat3 quarter = Mat3::rotate(kPi / 2);
    checkNear(quarter.apply({1, 0}), {0, 1});
    checkNear((quarter * quarter * quarter * quarter).apply({0.3f, -0.2f}), {0.3f, -0.2f});
}

TEST_CASE("products apply right to left") {
    // Scale first, then translate.
    const Mat3 m = Mat3::translate(100, 50) * Mat3::scale(10, 10);
    checkNear(m.apply({1, 1}), {110, 60});
    checkNear(m.apply({0, 0}), {100, 50});
}

TEST_CASE("a cubic Bezier passes through its end points and midpoint") {
    const Vec2 p0{0, 0}, p1{0, 1}, p2{1, 1}, p3{1, 0};
    checkNear(kitenge::bezier::point(p0, p1, p2, p3, 0.0f), p0);
    checkNear(kitenge::bezier::point(p0, p1, p2, p3, 1.0f), p3);
    checkNear(kitenge::bezier::point(p0, p1, p2, p3, 0.5f), {0.5f, 0.75f});
}
