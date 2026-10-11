#include "raster/Clip.h"

#include <cmath>
#include <utility>

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

enum class Edge { Left, Right, Top, Bottom };

bool inside(Vec2 p, Edge edge, const ClipRect& w) {
    switch (edge) {
        case Edge::Left:
            return p.x >= w.xmin;
        case Edge::Right:
            return p.x <= w.xmax;
        case Edge::Top:
            return p.y >= w.ymin;
        case Edge::Bottom:
            return p.y <= w.ymax;
    }
    return false;
}

// Where the segment a-b crosses the line of the given window edge.
Vec2 intersect(Vec2 a, Vec2 b, Edge edge, const ClipRect& w) {
    float t = 0.0f;
    switch (edge) {
        case Edge::Left:
            t = (w.xmin - a.x) / (b.x - a.x);
            return {w.xmin, a.y + (b.y - a.y) * t};
        case Edge::Right:
            t = (w.xmax - a.x) / (b.x - a.x);
            return {w.xmax, a.y + (b.y - a.y) * t};
        case Edge::Top:
            t = (w.ymin - a.y) / (b.y - a.y);
            return {a.x + (b.x - a.x) * t, w.ymin};
        case Edge::Bottom:
            t = (w.ymax - a.y) / (b.y - a.y);
            return {a.x + (b.x - a.x) * t, w.ymax};
    }
    return a;
}

}  // namespace

Polygon clipPolygon(const Polygon& poly, const ClipRect& window) {
    std::vector<Vec2> output = poly.points;
    for (Edge edge : {Edge::Left, Edge::Right, Edge::Top, Edge::Bottom}) {
        if (output.empty()) {
            break;
        }
        std::vector<Vec2> input;
        std::swap(input, output);

        // Walk each edge (prev -> curr) and apply the four Sutherland-Hodgman cases.
        Vec2 prev = input.back();
        for (Vec2 curr : input) {
            const bool currIn = inside(curr, edge, window);
            const bool prevIn = inside(prev, edge, window);
            if (currIn) {
                if (!prevIn) {
                    output.push_back(intersect(prev, curr, edge, window));  // out -> in
                }
                output.push_back(curr);  // in -> in (and the end of out -> in)
            } else if (prevIn) {
                output.push_back(intersect(prev, curr, edge, window));  // in -> out
            }
            prev = curr;  // out -> out adds nothing
        }
    }
    return Polygon{output};
}

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
