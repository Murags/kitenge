#include "raster/Raster.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace kitenge::raster {

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

}  // namespace kitenge::raster
