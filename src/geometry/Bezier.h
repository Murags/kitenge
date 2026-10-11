#pragma once

#include <vector>

#include "core/Vec2.h"
#include "geometry/Motif.h"

namespace kitenge::bezier {

// Point on the cubic curve p0..p3 at parameter t in [0, 1], using
// de Casteljau's algorithm: repeated linear interpolation between the
// control points.
constexpr Vec2 point(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    const Vec2 a = lerp(p0, p1, t);
    const Vec2 b = lerp(p1, p2, t);
    const Vec2 c = lerp(p2, p3, t);
    const Vec2 d = lerp(a, b, t);
    const Vec2 e = lerp(b, c, t);
    return lerp(d, e, t);
}

// Planned for Phase 3: turn a Bezier outline into a polygon with `steps`
// points per piece, so the curved motifs can be drawn and filled like any
// other polygon.
Polygon flatten(const BezierPath& path, int steps);

}  // namespace kitenge::bezier
