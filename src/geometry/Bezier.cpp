#include "geometry/Bezier.h"

#include <algorithm>

namespace kitenge::bezier {

Vec2 point(const std::vector<Vec2>& control, float t) {
    if (control.empty()) {
        return {};
    }
    // Repeatedly replace the control polygon by the points a fraction t along
    // each of its legs until one point is left.
    std::vector<Vec2> pts = control;
    for (std::size_t level = pts.size() - 1; level > 0; --level) {
        for (std::size_t i = 0; i < level; ++i) {
            pts[i] = lerp(pts[i], pts[i + 1], t);
        }
    }
    return pts.front();
}

std::vector<Vec2> sample(const std::vector<Vec2>& control, int steps) {
    steps = std::max(steps, 1);
    std::vector<Vec2> out;
    out.reserve(static_cast<std::size_t>(steps) + 1);
    for (int i = 0; i <= steps; ++i) {
        out.push_back(point(control, static_cast<float>(i) / static_cast<float>(steps)));
    }
    return out;
}

Polygon flatten(const BezierPath& path, int steps) {
    steps = std::max(steps, 1);
    Polygon poly;
    poly.points.push_back(path.start);
    Vec2 from = path.start;
    for (const CubicSegment& seg : path.segments) {
        // t = 0 is the previous piece's end point, already in the polygon.
        for (int i = 1; i <= steps; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(steps);
            poly.points.push_back(point(from, seg.control1, seg.control2, seg.end, t));
        }
        from = seg.end;
    }
    // A closed path ends on its start point, which the polygon already has.
    if (poly.points.size() > 1 && poly.points.back() == poly.points.front()) {
        poly.points.pop_back();
    }
    return poly;
}

}  // namespace kitenge::bezier
