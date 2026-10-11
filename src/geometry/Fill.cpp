#include "geometry/Fill.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace kitenge::fill {

namespace {

// One non-horizontal polygon edge in the edge table.
struct Edge {
    int yStart;     // first scan line the edge crosses
    int yEnd;       // one past the last scan line it crosses
    double x;       // x where the edge crosses the current scan line's centre
    double dxPerY;  // inverse slope: change in x per scan line
};

}  // namespace

void polygon(Framebuffer& fb, const Polygon& poly, Color fill) {
    const std::size_t n = poly.points.size();
    if (n < 3 || fb.width() == 0 || fb.height() == 0) {
        return;
    }

    // Build the edge table. Scan line y samples the pixel centres at y + 0.5,
    // and each edge covers the half-open range [top, bottom) so a shared
    // vertex is counted once.
    std::vector<Edge> table;
    for (std::size_t i = 0; i < n; ++i) {
        Vec2 a = poly.points[i];
        Vec2 b = poly.points[(i + 1) % n];
        if (a.y == b.y) {
            continue;  // horizontal edges never cross a scan line centre
        }
        if (a.y > b.y) {
            std::swap(a, b);
        }
        Edge e;
        e.yStart = static_cast<int>(std::ceil(a.y - 0.5f));
        e.yEnd = static_cast<int>(std::ceil(b.y - 0.5f));
        if (e.yStart >= e.yEnd) {
            continue;
        }
        e.dxPerY = static_cast<double>(b.x - a.x) / (b.y - a.y);
        e.x = a.x + (e.yStart + 0.5 - a.y) * e.dxPerY;
        table.push_back(e);
    }
    std::sort(table.begin(), table.end(),
              [](const Edge& l, const Edge& r) { return l.yStart < r.yStart; });

    std::vector<Edge> active;
    std::size_t next = 0;
    const int yFirst = table.empty() ? 0 : std::max(table.front().yStart, 0);
    // Edges that start above the canvas are stepped down to the first row.
    for (; next < table.size() && table[next].yStart < yFirst; ++next) {
        Edge e = table[next];
        if (e.yEnd > yFirst) {
            e.x += (yFirst - e.yStart) * e.dxPerY;
            active.push_back(e);
        }
    }

    for (int y = yFirst; y < fb.height(); ++y) {
        // Move edges starting on this scan line into the active list and drop
        // the ones that have ended.
        while (next < table.size() && table[next].yStart == y) {
            active.push_back(table[next++]);
        }
        active.erase(std::remove_if(active.begin(), active.end(),
                                    [y](const Edge& e) { return e.yEnd <= y; }),
                     active.end());
        if (active.empty()) {
            if (next == table.size()) {
                break;
            }
            continue;
        }

        std::sort(active.begin(), active.end(),
                  [](const Edge& l, const Edge& r) { return l.x < r.x; });

        // Even-odd rule: fill between each pair of crossings.
        for (std::size_t i = 0; i + 1 < active.size(); i += 2) {
            const int xFrom = std::max(static_cast<int>(std::ceil(active[i].x - 0.5)), 0);
            const int xTo =
                std::min(static_cast<int>(std::ceil(active[i + 1].x - 0.5)), fb.width());
            for (int x = xFrom; x < xTo; ++x) {
                fb.setPixel(x, y, fill);
            }
        }

        for (Edge& e : active) {
            e.x += e.dxPerY;
        }
    }
}

}  // namespace kitenge::fill
