#include <cmath>

#include "raster/Raster.h"

namespace kitenge::raster {

namespace {

// Region outcodes. y points down, so "above" means y < ymin.
constexpr int kInside = 0;
constexpr int kLeft = 1;
constexpr int kRight = 2;
constexpr int kAbove = 4;
constexpr int kBelow = 8;

int outcode(double x, double y, int xmin, int ymin, int xmax, int ymax) {
    int code = kInside;
    if (x < xmin) {
        code |= kLeft;
    } else if (x > xmax) {
        code |= kRight;
    }
    if (y < ymin) {
        code |= kAbove;
    } else if (y > ymax) {
        code |= kBelow;
    }
    return code;
}

}  // namespace

bool clipLine(int& x0, int& y0, int& x1, int& y1, int xmin, int ymin, int xmax, int ymax) {
    // Work in doubles so repeated intersections do not accumulate rounding.
    double ax = x0, ay = y0, bx = x1, by = y1;
    int codeA = outcode(ax, ay, xmin, ymin, xmax, ymax);
    int codeB = outcode(bx, by, xmin, ymin, xmax, ymax);

    while (true) {
        if ((codeA | codeB) == 0) {
            break;  // both ends inside: trivially accept
        }
        if ((codeA & codeB) != 0) {
            return false;  // both ends outside the same edge: trivially reject
        }

        // Move the end point that is outside onto the edge it crosses.
        const int out = codeA != 0 ? codeA : codeB;
        double x = 0.0;
        double y = 0.0;
        if (out & kAbove) {
            x = ax + (bx - ax) * (ymin - ay) / (by - ay);
            y = ymin;
        } else if (out & kBelow) {
            x = ax + (bx - ax) * (ymax - ay) / (by - ay);
            y = ymax;
        } else if (out & kRight) {
            y = ay + (by - ay) * (xmax - ax) / (bx - ax);
            x = xmax;
        } else {
            y = ay + (by - ay) * (xmin - ax) / (bx - ax);
            x = xmin;
        }

        if (out == codeA) {
            ax = x;
            ay = y;
            codeA = outcode(ax, ay, xmin, ymin, xmax, ymax);
        } else {
            bx = x;
            by = y;
            codeB = outcode(bx, by, xmin, ymin, xmax, ymax);
        }
    }

    x0 = static_cast<int>(std::lround(ax));
    y0 = static_cast<int>(std::lround(ay));
    x1 = static_cast<int>(std::lround(bx));
    y1 = static_cast<int>(std::lround(by));
    return true;
}

}  // namespace kitenge::raster
