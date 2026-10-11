#pragma once

#include "geometry/Motif.h"

namespace kitenge::raster {

// An axis-aligned clip window in canvas coordinates.
struct ClipRect {
    float xmin = 0.0f;
    float ymin = 0.0f;
    float xmax = 0.0f;
    float ymax = 0.0f;
};

// Sutherland-Hodgman polygon clipping: clips `poly` against each edge of the
// window in turn and returns what is left, which may be empty. The result is
// still one closed polygon, so it can be filled like the original. Line
// clipping (Cohen-Sutherland) is raster::clipLine in Raster.h.
Polygon clipPolygon(const Polygon& poly, const ClipRect& window);

}  // namespace kitenge::raster
