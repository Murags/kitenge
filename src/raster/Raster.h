#pragma once

#include "core/Color.h"
#include "core/Framebuffer.h"

namespace kitenge::raster {

// Planned primitives for Phase 2. Each one writes pixels only through
// Framebuffer::setPixel. They are declared here so the other modules can code
// against them now; definitions arrive with the matching lab.

// Digital differential analyser line from (x0, y0) to (x1, y1), inclusive.
void lineDDA(Framebuffer& fb, int x0, int y0, int x1, int y1, Color color);

// Bresenham integer line from (x0, y0) to (x1, y1), inclusive.
void lineBresenham(Framebuffer& fb, int x0, int y0, int x1, int y1, Color color);

// Midpoint circle outline centred on (cx, cy).
void circle(Framebuffer& fb, int cx, int cy, int radius, Color color);

// Midpoint ellipse outline centred on (cx, cy) with radii rx and ry.
void ellipse(Framebuffer& fb, int cx, int cy, int rx, int ry, Color color);

// Cohen-Sutherland clip of a line against the rectangle [xmin, xmax] x [ymin, ymax].
// Updates the end points in place and returns false when nothing is visible.
bool clipLine(int& x0, int& y0, int& x1, int& y1, int xmin, int ymin, int xmax, int ymax);

}  // namespace kitenge::raster
