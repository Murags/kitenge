#pragma once

#include "core/Color.h"
#include "core/Framebuffer.h"
#include "geometry/Motif.h"

namespace kitenge::fill {

// Planned for Phase 2. Both write pixels only through Framebuffer::setPixel.

// Boundary fill from the seed (x, y): paints outward until it reaches pixels
// of `boundary` colour. Used to colour the enclosed regions of a motif after
// its outline has been drawn.
void boundaryFill(Framebuffer& fb, int x, int y, Color fill, Color boundary);

// Scan-line fill of a polygon given in pixel coordinates.
void polygon(Framebuffer& fb, const Polygon& poly, Color fill);

}  // namespace kitenge::fill
