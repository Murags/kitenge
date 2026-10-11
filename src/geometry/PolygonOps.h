#pragma once

#include "core/Vec2.h"
#include "geometry/Motif.h"

namespace kitenge::polygon {

// Basic polygon operations used by the fills, the motif library and tests.

// Shoelace formula. Positive when the points run clockwise on screen
// (y pointing down), negative when they run anticlockwise.
float signedArea(const Polygon& poly);

float area(const Polygon& poly);

// Centre of mass of the polygon's area. Falls back to the average of the
// points for degenerate (zero-area) polygons.
Vec2 centroid(const Polygon& poly);

// Even-odd point-in-polygon test by casting a ray towards +x and counting
// the edges it crosses.
bool contains(const Polygon& poly, Vec2 p);

// True when every turn goes the same way, i.e. the polygon has no dents.
bool isConvex(const Polygon& poly);

struct Bounds {
    Vec2 min;
    Vec2 max;
};

Bounds bounds(const Polygon& poly);

// A regular polygon with `sides` corners on a circle, starting at `angle`
// radians (0 points along +x). Used for stars, diamonds and approximated
// circles in the motifs.
Polygon regular(Vec2 centre, float radius, int sides, float angle = 0.0f);

// A star with `points` tips alternating between the outer and inner radius.
Polygon star(Vec2 centre, float outerRadius, float innerRadius, int points, float angle = 0.0f);

}  // namespace kitenge::polygon
