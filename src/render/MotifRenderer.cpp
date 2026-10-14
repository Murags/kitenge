#include "render/MotifRenderer.h"

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <variant>

#include "geometry/Bezier.h"
#include "geometry/Fill.h"
#include "geometry/Spline.h"
#include "raster/Clip.h"
#include "raster/Raster.h"

namespace kitenge::render {

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

int toPixel(float v) {
    return static_cast<int>(std::lround(v));
}

Polygon transformed(const Polygon& poly, const Mat3& m) {
    Polygon out;
    out.points.reserve(poly.points.size());
    for (Vec2 p : poly.points) {
        out.points.push_back(m.apply(p));
    }
    return out;
}

// How many pixels one tile-local unit becomes (the uniform part of the scale).
float pixelsPerUnit(const Mat3& m) {
    const float det = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
    return std::sqrt(std::fabs(det));
}

// Curve samples per Bezier piece or spline span: about one every 4 pixels.
int curveSteps(float sizeInPixels) {
    return std::clamp(static_cast<int>(sizeInPixels / 4.0f), 4, 48);
}

Color paletteColor(const std::vector<Color>& palette, int index) {
    if (palette.empty()) {
        return Color();
    }
    return palette[static_cast<std::size_t>(index) % palette.size()];
}

// A circle or ellipse after the transform. Rotations by multiples of 90
// degrees and reflections keep it axis-aligned, so it can still go to the
// midpoint algorithms with its radii swapped if needed; anything else is
// approximated by a polygon.
struct ScreenEllipse {
    bool axisAligned = false;
    Vec2 centre;
    float rx = 0.0f;
    float ry = 0.0f;
};

ScreenEllipse toScreen(Vec2 centre, float rx, float ry, const Mat3& m) {
    ScreenEllipse e;
    e.centre = m.apply(centre);
    const Vec2 ax = m.apply(centre + Vec2(rx, 0)) - e.centre;
    const Vec2 ay = m.apply(centre + Vec2(0, ry)) - e.centre;
    const float eps = 1e-3f * std::max(length(ax), length(ay)) + 1e-6f;
    if (std::fabs(ax.y) < eps && std::fabs(ay.x) < eps) {
        e = {true, e.centre, std::fabs(ax.x), std::fabs(ay.y)};
    } else if (std::fabs(ax.x) < eps && std::fabs(ay.y) < eps) {
        e = {true, e.centre, std::fabs(ay.x), std::fabs(ax.y)};
    }
    return e;
}

Polygon ellipsePolygon(Vec2 centre, float rx, float ry, const Mat3& m) {
    const int n = std::max(curveSteps(kTwoPi * std::max(rx, ry) * pixelsPerUnit(m)) * 2, 16);
    Polygon local;
    for (int i = 0; i < n; ++i) {
        const float t = kTwoPi * static_cast<float>(i) / static_cast<float>(n);
        local.points.push_back(centre + Vec2(rx * std::cos(t), ry * std::sin(t)));
    }
    return transformed(local, m);
}

void drawRound(Framebuffer& fb, Vec2 centre, float rx, float ry, const Mat3& m, int fillIndex,
               int strokeIndex, const std::vector<Color>& palette, const RenderOptions& opt) {
    const ScreenEllipse e = toScreen(centre, rx, ry, m);
    if (!e.axisAligned) {
        const Polygon poly = ellipsePolygon(centre, rx, ry, m);
        if (opt.fill && fillIndex != kNoColor) {
            fillPolygon(fb, poly, paletteColor(palette, fillIndex));
        }
        if (opt.outline && strokeIndex != kNoColor) {
            drawPolygonOutline(fb, poly, paletteColor(palette, strokeIndex));
        }
        return;
    }

    const int cx = toPixel(e.centre.x);
    const int cy = toPixel(e.centre.y);
    const int prx = toPixel(e.rx);
    const int pry = toPixel(e.ry);
    const bool isCircle = prx == pry;
    if (opt.fill && fillIndex != kNoColor) {
        const Color c = paletteColor(palette, fillIndex);
        if (isCircle) {
            raster::fillCircle(fb, cx, cy, prx, c);
        } else {
            raster::fillEllipse(fb, cx, cy, prx, pry, c);
        }
    }
    if (opt.outline && strokeIndex != kNoColor) {
        const Color c = paletteColor(palette, strokeIndex);
        if (isCircle) {
            raster::circle(fb, cx, cy, prx, c);
        } else {
            raster::ellipse(fb, cx, cy, prx, pry, c);
        }
    }
}

void drawOutlined(Framebuffer& fb, const Polygon& screen, int fillIndex, int strokeIndex,
                  const std::vector<Color>& palette, const RenderOptions& opt) {
    if (opt.fill && fillIndex != kNoColor) {
        fillPolygon(fb, screen, paletteColor(palette, fillIndex));
    }
    if (opt.outline && strokeIndex != kNoColor) {
        drawPolygonOutline(fb, screen, paletteColor(palette, strokeIndex));
    }
}

}  // namespace

void drawLine(Framebuffer& fb, Vec2 a, Vec2 b, Color color) {
    int x0 = toPixel(a.x), y0 = toPixel(a.y), x1 = toPixel(b.x), y1 = toPixel(b.y);
    if (!raster::clipLine(x0, y0, x1, y1, 0, 0, fb.width() - 1, fb.height() - 1)) {
        return;
    }
    raster::lineBresenham(fb, x0, y0, x1, y1, color);
}

void drawPolygonOutline(Framebuffer& fb, const Polygon& poly, Color color) {
    const std::size_t n = poly.points.size();
    for (std::size_t i = 0; i < n; ++i) {
        drawLine(fb, poly.points[i], poly.points[(i + 1) % n], color);
    }
}

void fillPolygon(Framebuffer& fb, const Polygon& poly, Color color) {
    const raster::ClipRect canvas{0.0f, 0.0f, static_cast<float>(fb.width()),
                                  static_cast<float>(fb.height())};
    fill::polygon(fb, raster::clipPolygon(poly, canvas), color);
}

void drawMotif(Framebuffer& fb, const Motif& motif, const Mat3& transform,
               const std::vector<Color>& palette, const RenderOptions& options) {
    const float unit = pixelsPerUnit(transform);
    for (const Shape& shape : motif.shapes) {
        std::visit(
            [&](const auto& g) {
                using T = std::decay_t<decltype(g)>;
                if constexpr (std::is_same_v<T, Segment>) {
                    if (options.outline && shape.strokeIndex != kNoColor) {
                        drawLine(fb, transform.apply(g.a), transform.apply(g.b),
                                 paletteColor(palette, shape.strokeIndex));
                    }
                } else if constexpr (std::is_same_v<T, Polygon>) {
                    drawOutlined(fb, transformed(g, transform), shape.fillIndex, shape.strokeIndex,
                                 palette, options);
                } else if constexpr (std::is_same_v<T, Circle>) {
                    drawRound(fb, g.centre, g.radius, g.radius, transform, shape.fillIndex,
                              shape.strokeIndex, palette, options);
                } else if constexpr (std::is_same_v<T, Ellipse>) {
                    drawRound(fb, g.centre, g.rx, g.ry, transform, shape.fillIndex,
                              shape.strokeIndex, palette, options);
                } else if constexpr (std::is_same_v<T, BezierPath>) {
                    const Polygon local = bezier::flatten(g, curveSteps(unit * 0.5f));
                    drawOutlined(fb, transformed(local, transform), shape.fillIndex,
                                 shape.strokeIndex, palette, options);
                } else if constexpr (std::is_same_v<T, SplinePath>) {
                    const Polygon local = spline::flatten(g, curveSteps(unit * 0.25f));
                    drawOutlined(fb, transformed(local, transform), shape.fillIndex,
                                 shape.strokeIndex, palette, options);
                }
            },
            shape.geometry);
    }
}

}  // namespace kitenge::render
