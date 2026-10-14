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

// Midpoint ellipse over the first quadrant, from (0, ry) to (rx, 0).
// Region 1 steps in x while the slope is shallower than -1, region 2 steps
// in y for the rest. Squared radii are kept in 64-bit integers so large
// ellipses cannot overflow.
template <typename Visit>
void midpointEllipse(int rx, int ry, Visit visit) {
    const long long rx2 = static_cast<long long>(rx) * rx;
    const long long ry2 = static_cast<long long>(ry) * ry;
    int x = 0;
    int y = ry;
    long long px = 0;
    long long py = 2 * rx2 * y;

    // Decision parameters are scaled by 4 so they stay integers.
    long long p1 = 4 * ry2 - 4 * rx2 * ry + rx2;
    visit(x, y);
    while (px < py) {
        ++x;
        px += 2 * ry2;
        if (p1 < 0) {
            p1 += 4 * (ry2 + px);
        } else {
            --y;
            py -= 2 * rx2;
            p1 += 4 * (ry2 + px - py);
        }
        visit(x, y);
    }

    long long p2 = ry2 * (2 * x + 1) * (2 * x + 1) + 4 * rx2 * (y - 1) * (y - 1) - 4 * rx2 * ry2;
    while (y > 0) {
        --y;
        py -= 2 * rx2;
        if (p2 > 0) {
            p2 += 4 * (rx2 - py);
        } else {
            ++x;
            px += 2 * ry2;
            p2 += 4 * (rx2 - py + px);
        }
        visit(x, y);
    }
}

}  // namespace

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

void ellipse(Framebuffer& fb, int cx, int cy, int rx, int ry, Color color) {
    if (rx < 0 || ry < 0) {
        return;
    }
    if (rx == 0 || ry == 0) {
        lineBresenham(fb, cx - rx, cy - ry, cx + rx, cy + ry, color);
        return;
    }
    midpointEllipse(rx, ry, [&](int x, int y) {
        fb.setPixel(cx + x, cy + y, color);
        fb.setPixel(cx - x, cy + y, color);
        fb.setPixel(cx + x, cy - y, color);
        fb.setPixel(cx - x, cy - y, color);
    });
}

void fillEllipse(Framebuffer& fb, int cx, int cy, int rx, int ry, Color color) {
    if (rx < 0 || ry < 0) {
        return;
    }
    if (rx == 0 || ry == 0) {
        lineBresenham(fb, cx - rx, cy - ry, cx + rx, cy + ry, color);
        return;
    }
    midpointEllipse(rx, ry, [&](int x, int y) {
        span(fb, cy + y, cx - x, cx + x, color);
        span(fb, cy - y, cx - x, cx + x, color);
    });
}

}  // namespace kitenge::raster
