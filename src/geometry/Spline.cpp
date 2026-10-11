#include "geometry/Spline.h"

#include <algorithm>
#include <cstddef>

namespace kitenge::spline {

namespace {

// Catmull-Rom basis for the piece from p1 to p2.
Vec2 catmullRomPoint(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    return ((p1 * 2.0f) + (p2 - p0) * t + (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 +
            (-p0 + p1 * 3.0f - p2 * 3.0f + p3) * t3) *
           0.5f;
}

// Uniform cubic B-spline basis functions; they always sum to 1.
Vec2 bSplinePoint(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    const float u = 1.0f - t;
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float b0 = u * u * u / 6.0f;
    const float b1 = (3.0f * t3 - 6.0f * t2 + 4.0f) / 6.0f;
    const float b2 = (-3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f) / 6.0f;
    const float b3 = t3 / 6.0f;
    return p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;
}

// Walks every window of four control points and samples the piece between
// them. Open curves clamp the window at the ends by repeating the end points.
template <typename Basis>
std::vector<Vec2> sampleCurve(const std::vector<Vec2>& pts, int steps, bool closed, Basis basis) {
    std::vector<Vec2> out;
    const int n = static_cast<int>(pts.size());
    if (n < 2) {
        return pts;
    }
    steps = std::max(steps, 1);
    auto at = [&](int i) {
        if (closed) {
            return pts[static_cast<std::size_t>(((i % n) + n) % n)];
        }
        return pts[static_cast<std::size_t>(std::clamp(i, 0, n - 1))];
    };

    const int pieces = closed ? n : n - 1;
    for (int seg = 0; seg < pieces; ++seg) {
        // Closed loops skip t = 1, which is the next piece's t = 0.
        const int last = (closed || seg + 1 < pieces) ? steps - 1 : steps;
        for (int i = 0; i <= last; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(steps);
            out.push_back(basis(at(seg - 1), at(seg), at(seg + 1), at(seg + 2), t));
        }
    }
    return out;
}

}  // namespace

std::vector<Vec2> catmullRom(const std::vector<Vec2>& points, int steps, bool closed) {
    return sampleCurve(points, steps, closed, catmullRomPoint);
}

std::vector<Vec2> bSpline(const std::vector<Vec2>& points, int steps, bool closed) {
    return sampleCurve(points, steps, closed, bSplinePoint);
}

Polygon flatten(const SplinePath& path, int steps) {
    if (path.kind == SplineKind::CatmullRom) {
        return Polygon{catmullRom(path.points, steps, true)};
    }
    return Polygon{bSpline(path.points, steps, true)};
}

}  // namespace kitenge::spline
