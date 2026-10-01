#pragma once

#include <string>
#include <variant>
#include <vector>

#include "core/Vec2.h"

namespace kitenge {

// Motifs are described in tile-local coordinates: the tile is the square
// from (-0.5, -0.5) to (0.5, 0.5), centred on the origin with y pointing down.
// The symmetry engine scales, rotates, mirrors and places each copy, so a
// motif never needs to know the tile size or where it ends up on the canvas.

// A straight stroke, such as the short bars inside the motifs.
struct Segment {
    Vec2 a;
    Vec2 b;
};

// A closed polygon. The last point joins back to the first.
struct Polygon {
    std::vector<Vec2> points;
};

struct Circle {
    Vec2 centre;
    float radius = 0.0f;
};

struct Ellipse {
    Vec2 centre;
    float rx = 0.0f;
    float ry = 0.0f;
};

// One cubic Bezier piece, continuing from the end of the previous piece.
struct CubicSegment {
    Vec2 control1;
    Vec2 control2;
    Vec2 end;
};

// A closed outline made of cubic Bezier pieces, e.g. the leaf motif.
// It starts at `start` and the last piece should end back on `start`.
struct BezierPath {
    Vec2 start;
    std::vector<CubicSegment> segments;
};

using Primitive = std::variant<Segment, Polygon, Circle, Ellipse, BezierPath>;

// No palette entry: used to leave a shape unfilled or without an outline.
constexpr int kNoColor = -1;

// One part of a motif. Colours are indices into the current palette, so
// changing the palette recolours every motif without touching its geometry.
struct Shape {
    Primitive geometry;
    int fillIndex = kNoColor;
    int strokeIndex = 0;
};

struct Motif {
    std::string name;
    std::vector<Shape> shapes;
};

}  // namespace kitenge
