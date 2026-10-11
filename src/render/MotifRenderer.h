#pragma once

#include <vector>

#include "core/Color.h"
#include "core/Framebuffer.h"
#include "geometry/Motif.h"
#include "symmetry/Mat3.h"

namespace kitenge::render {

enum class LineAlgorithm { Bresenham, DDA };

struct RenderOptions {
    bool fill = true;
    bool outline = true;
    LineAlgorithm lines = LineAlgorithm::Bresenham;
};

// A straight line in canvas coordinates: Cohen-Sutherland clips it to the
// framebuffer, then the chosen line algorithm draws what is left.
void drawLine(Framebuffer& fb, Vec2 a, Vec2 b, Color color,
              LineAlgorithm algorithm = LineAlgorithm::Bresenham);

// Closed outline of a polygon in canvas coordinates.
void drawPolygonOutline(Framebuffer& fb, const Polygon& poly, Color color,
                        LineAlgorithm algorithm = LineAlgorithm::Bresenham);

// Sutherland-Hodgman clips the polygon to the framebuffer, then the
// scan-line fill colours it.
void fillPolygon(Framebuffer& fb, const Polygon& poly, Color color);

// Draws one motif through `transform` (tile-local units to canvas pixels).
// Each shape is filled first and then outlined, in the motif's order, with
// colours looked up in `palette`. Curves are flattened with enough points
// for their size on screen.
void drawMotif(Framebuffer& fb, const Motif& motif, const Mat3& transform,
               const std::vector<Color>& palette, const RenderOptions& options = {});

}  // namespace kitenge::render
