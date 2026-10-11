#include "geometry/PolygonOps.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace kitenge::polygon {

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

float cross(Vec2 a, Vec2 b) {
    return a.x * b.y - a.y * b.x;
}

}  // namespace

float signedArea(const Polygon& poly) {
    const std::size_t n = poly.points.size();
    float twice = 0.0f;
    for (std::size_t i = 0; i < n; ++i) {
        twice += cross(poly.points[i], poly.points[(i + 1) % n]);
    }
    return twice / 2.0f;
}

float area(const Polygon& poly) {
    return std::fabs(signedArea(poly));
}

Vec2 centroid(const Polygon& poly) {
    const std::size_t n = poly.points.size();
    if (n == 0) {
        return {};
    }
    const float a = signedArea(poly);
    if (std::fabs(a) < 1e-12f) {
        Vec2 sum;
        for (Vec2 p : poly.points) {
            sum += p;
        }
        return sum / static_cast<float>(n);
    }
    Vec2 c;
    for (std::size_t i = 0; i < n; ++i) {
        const Vec2 p = poly.points[i];
        const Vec2 q = poly.points[(i + 1) % n];
        c += (p + q) * cross(p, q);
    }
    return c / (6.0f * a);
}

bool contains(const Polygon& poly, Vec2 p) {
    const std::size_t n = poly.points.size();
    bool inside = false;
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const Vec2 a = poly.points[i];
        const Vec2 b = poly.points[j];
        // Does edge a-b straddle the ray's y, and is the crossing to the right of p?
        if ((a.y > p.y) != (b.y > p.y)) {
            const float xCross = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
            if (p.x < xCross) {
                inside = !inside;
            }
        }
    }
    return inside;
}

bool isConvex(const Polygon& poly) {
    const std::size_t n = poly.points.size();
    if (n < 3) {
        return false;
    }
    int sign = 0;
    for (std::size_t i = 0; i < n; ++i) {
        const Vec2 a = poly.points[i];
        const Vec2 b = poly.points[(i + 1) % n];
        const Vec2 c = poly.points[(i + 2) % n];
        const float turn = cross(b - a, c - b);
        if (std::fabs(turn) < 1e-9f) {
            continue;
        }
        const int s = turn > 0 ? 1 : -1;
        if (sign != 0 && s != sign) {
            return false;
        }
        sign = s;
    }
    return sign != 0;
}

Bounds bounds(const Polygon& poly) {
    if (poly.points.empty()) {
        return {};
    }
    Bounds b{poly.points.front(), poly.points.front()};
    for (Vec2 p : poly.points) {
        b.min = {std::min(b.min.x, p.x), std::min(b.min.y, p.y)};
        b.max = {std::max(b.max.x, p.x), std::max(b.max.y, p.y)};
    }
    return b;
}

Polygon regular(Vec2 centre, float radius, int sides, float angle) {
    Polygon poly;
    for (int i = 0; i < sides; ++i) {
        const float t = angle + kTwoPi * static_cast<float>(i) / static_cast<float>(sides);
        poly.points.push_back(centre + Vec2(std::cos(t), std::sin(t)) * radius);
    }
    return poly;
}

Polygon star(Vec2 centre, float outerRadius, float innerRadius, int points, float angle) {
    Polygon poly;
    for (int i = 0; i < 2 * points; ++i) {
        const float t = angle + kTwoPi * static_cast<float>(i) / static_cast<float>(2 * points);
        const float r = i % 2 == 0 ? outerRadius : innerRadius;
        poly.points.push_back(centre + Vec2(std::cos(t), std::sin(t)) * r);
    }
    return poly;
}

}  // namespace kitenge::polygon
