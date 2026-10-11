#pragma once

#include <vector>

#include "core/Vec2.h"
#include "geometry/Motif.h"

namespace kitenge::spline {

// Both curves are piecewise cubics built from four consecutive control
// points at a time, with `steps` samples per piece. When `closed` is true the
// control points wrap around and the result is a closed loop (the last
// sample is not repeated).

// Catmull-Rom spline: passes through every control point, with the tangent
// at each point parallel to the line joining its neighbours.
std::vector<Vec2> catmullRom(const std::vector<Vec2>& points, int steps, bool closed);

// Uniform cubic B-spline: smoother (C2) but only approximates the control
// points instead of passing through them.
std::vector<Vec2> bSpline(const std::vector<Vec2>& points, int steps, bool closed);

// Samples a SplinePath from a motif into a closed polygon.
Polygon flatten(const SplinePath& path, int steps);

}  // namespace kitenge::spline
