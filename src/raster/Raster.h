#pragma once

#include "core/Color.h"
#include "core/Framebuffer.h"

namespace kitenge::raster {

// The low-level primitives every shape on the canvas is built from. Each one
// writes pixels only through Framebuffer::setPixel, which ignores pixels that
// fall outside the buffer.

// Bresenham integer line from (x0, y0) to (x1, y1), inclusive. This is the
// only line algorithm in the project: every straight edge and stroke uses it.
void lineBresenham(Framebuffer& fb, int x0, int y0, int x1, int y1, Color color);

// Midpoint circle outline centred on (cx, cy).
void circle(Framebuffer& fb, int cx, int cy, int radius, Color color);

// Solid disc: the same midpoint stepping, but each pair of symmetric points
// is joined by a horizontal span.
void fillCircle(Framebuffer& fb, int cx, int cy, int radius, Color color);

// Midpoint ellipse outline centred on (cx, cy) with radii rx and ry.
void ellipse(Framebuffer& fb, int cx, int cy, int rx, int ry, Color color);

// Solid ellipse built from the midpoint ellipse points joined by spans.
void fillEllipse(Framebuffer& fb, int cx, int cy, int rx, int ry, Color color);

// Cohen-Sutherland clip of a line against the rectangle [xmin, xmax] x [ymin, ymax].
// Updates the end points in place and returns false when nothing is visible.
bool clipLine(int& x0, int& y0, int& x1, int& y1, int xmin, int ymin, int xmax, int ymax);

}  // namespace kitenge::raster
