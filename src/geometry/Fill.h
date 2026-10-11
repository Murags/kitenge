#pragma once

#include "core/Color.h"
#include "core/Framebuffer.h"
#include "geometry/Motif.h"

namespace kitenge::fill {

// Area fills. All of them write pixels only through Framebuffer::setPixel.

// Scan-line fill of a polygon given in canvas pixel coordinates, using an
// edge table and an active edge list with the even-odd rule. A pixel is
// filled when its centre (x + 0.5, y + 0.5) is inside the polygon.
void polygon(Framebuffer& fb, const Polygon& poly, Color fill);

// Boundary fill from the seed (x, y): paints outward (4-connected) until it
// reaches pixels of `boundary` colour. Used to colour the enclosed regions of
// a motif after its outline has been drawn.
void boundaryFill(Framebuffer& fb, int x, int y, Color fill, Color boundary);

// Flood fill from the seed (x, y): replaces the connected region of pixels
// that have the seed's current colour.
void floodFill(Framebuffer& fb, int x, int y, Color fill);

}  // namespace kitenge::fill
