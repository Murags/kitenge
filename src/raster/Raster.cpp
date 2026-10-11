#include "raster/Raster.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace kitenge::raster {

namespace {

void span(Framebuffer& fb, int y, int xFrom, int xTo, Color color) {
    for (int x = xFrom; x <= xTo; ++x) {
        fb.setPixel(x, y, color);
    }
}

// Midpoint circle stepping over the second octant (x from 0 up to y).
// `visit(x, y)` is called once per step; callers mirror it into the other
// seven octants.
template <typename Visit>
void midpointCircle(int radius, Visit visit) {
    int x = 0;
    int y = radius;
    int p = 1 - radius;
    visit(x, y);
    while (x < y) {
        ++x;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            --y;
            p += 2 * (x - y) + 1;
        }
        visit(x, y);
    }
}

}  // namespace

void lineDDA(Framebuffer& fb, int x0, int y0, int x1, int y1, Color color) {
    const int dx = x1 - x0;
    const int dy = y1 - y0;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) {
        fb.setPixel(x0, y0, color);
        return;
    }

    // Step one pixel along the major axis and a fraction along the other.
    const double xInc = static_cast<double>(dx) / steps;
    const double yInc = static_cast<double>(dy) / steps;
    double x = x0;
    double y = y0;
    for (int i = 0; i <= steps; ++i) {
        fb.setPixel(static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y)), color);
        x += xInc;
        y += yInc;
    }
}

void lineBresenham(Framebuffer& fb, int x0, int y0, int x1, int y1, Color color) {
    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;

    if (dx >= dy) {
        // Gentle slope: x always advances, the decision parameter says when y does.
        int p = 2 * dy - dx;
        int y = y0;
        for (int x = x0;; x += sx) {
            fb.setPixel(x, y, color);
            if (x == x1) {
                break;
            }
            if (p >= 0) {
                y += sy;
                p -= 2 * dx;
            }
            p += 2 * dy;
        }
    } else {
        // Steep slope: the same algorithm with the roles of x and y swapped.
        int p = 2 * dx - dy;
        int x = x0;
        for (int y = y0;; y += sy) {
            fb.setPixel(x, y, color);
            if (y == y1) {
                break;
            }
            if (p >= 0) {
                x += sx;
                p -= 2 * dy;
            }
            p += 2 * dx;
        }
    }
}

void circle(Framebuffer& fb, int cx, int cy, int radius, Color color) {
    if (radius < 0) {
        return;
    }
    midpointCircle(radius, [&](int x, int y) {
        fb.setPixel(cx + x, cy + y, color);
        fb.setPixel(cx - x, cy + y, color);
        fb.setPixel(cx + x, cy - y, color);
        fb.setPixel(cx - x, cy - y, color);
        fb.setPixel(cx + y, cy + x, color);
        fb.setPixel(cx - y, cy + x, color);
        fb.setPixel(cx + y, cy - x, color);
        fb.setPixel(cx - y, cy - x, color);
    });
}

void fillCircle(Framebuffer& fb, int cx, int cy, int radius, Color color) {
    if (radius < 0) {
        return;
    }
    midpointCircle(radius, [&](int x, int y) {
        span(fb, cy + y, cx - x, cx + x, color);
        span(fb, cy - y, cx - x, cx + x, color);
        span(fb, cy + x, cx - y, cx + y, color);
        span(fb, cy - x, cx - y, cx + y, color);
    });
}

}  // namespace kitenge::raster
