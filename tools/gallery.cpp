// Renders one image per Checkpoint 1 technique into a folder of BMP files,
// using only our own algorithms. No window or graphics library is involved.
//
//   kitenge_gallery [output-folder]     (default: gallery)

#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "core/Framebuffer.h"
#include "core/PatternSettings.h"
#include "geometry/Bezier.h"
#include "geometry/Fill.h"
#include "geometry/PolygonOps.h"
#include "geometry/Spline.h"
#include "motifs/Export.h"
#include "motifs/MotifLibrary.h"
#include "raster/Clip.h"
#include "raster/Raster.h"
#include "render/MotifRenderer.h"
#include "render/PatternRenderer.h"

using namespace kitenge;

namespace {

constexpr float kPi = 3.14159265358979f;

const PatternSettings kDefaults;
const Color kPaper = kDefaults.background;
const Color kGreen = kDefaults.palette[0];
const Color kSaffron = kDefaults.palette[1];
const Color kTerracotta = kDefaults.palette[2];
const Color kCharcoal = kDefaults.palette[3];
const Color kGuide(200, 190, 175);
const Color kBlue(40, 90, 200);

std::string outDir = "gallery";

void save(const Framebuffer& fb, const std::string& name) {
    const std::string path = outDir + "/" + name + ".bmp";
    if (writeBmp(fb, path)) {
        std::printf("wrote %s\n", path.c_str());
    } else {
        std::fprintf(stderr, "could not write %s\n", path.c_str());
    }
}

// Copies `src` into `dst` at (ox, oy), each pixel blown up to a k x k block,
// so the individual pixels chosen by an algorithm are visible.
void magnify(const Framebuffer& src, Framebuffer& dst, int ox, int oy, int k) {
    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.width(); ++x) {
            const Color c = src.getPixel(x, y);
            for (int j = 0; j < k - 1; ++j) {
                for (int i = 0; i < k - 1; ++i) {
                    dst.setPixel(ox + x * k + i, oy + y * k + j, c);
                }
            }
        }
    }
}

void frame(Framebuffer& fb, int x0, int y0, int x1, int y1, Color c) {
    raster::lineBresenham(fb, x0, y0, x1, y0, c);
    raster::lineBresenham(fb, x1, y0, x1, y1, c);
    raster::lineBresenham(fb, x1, y1, x0, y1, c);
    raster::lineBresenham(fb, x0, y1, x0, y0, c);
}

void dot(Framebuffer& fb, Vec2 p, Color c) {
    raster::fillCircle(fb, static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y)),
                       4, c);
}

// 1. DDA and Bresenham: a fan of lines in every octant, plus a magnified
//    shallow line from each algorithm.
void lines() {
    Framebuffer fb(960, 420);
    fb.clear(kPaper);
    for (int panel = 0; panel < 2; ++panel) {
        const int cx = 160 + panel * 320;
        const int cy = 210;
        for (int i = 0; i < 32; ++i) {
            const float t = 2.0f * kPi * static_cast<float>(i) / 32.0f;
            const int x = cx + static_cast<int>(std::lround(150 * std::cos(t)));
            const int y = cy + static_cast<int>(std::lround(150 * std::sin(t)));
            if (panel == 0) {
                raster::lineDDA(fb, cx, cy, x, y, i % 2 ? kGreen : kTerracotta);
            } else {
                raster::lineBresenham(fb, cx, cy, x, y, i % 2 ? kGreen : kBlue);
            }
        }
    }
    Framebuffer small(21, 9);
    small.clear(kPaper);
    raster::lineDDA(small, 0, 1, 20, 7, kTerracotta);
    magnify(small, fb, 680, 40, 12);
    small.clear(kPaper);
    raster::lineBresenham(small, 0, 1, 20, 7, kBlue);
    magnify(small, fb, 680, 260, 12);
    save(fb, "01-lines-dda-bresenham");
}

// 2. Midpoint circles and ellipses, outline and filled, plus a magnified circle.
void circlesAndEllipses() {
    Framebuffer fb(960, 420);
    fb.clear(kPaper);
    for (int r = 20; r <= 160; r += 20) {
        raster::circle(fb, 180, 210, r, r % 40 ? kGreen : kTerracotta);
    }
    for (int i = 0; i < 6; ++i) {
        raster::ellipse(fb, 500, 210, 150 - i * 22, 40 + i * 22, i % 2 ? kBlue : kCharcoal);
    }
    raster::fillEllipse(fb, 760, 110, 90, 50, kSaffron);
    raster::ellipse(fb, 760, 110, 90, 50, kCharcoal);
    raster::fillCircle(fb, 760, 290, 70, kTerracotta);
    raster::fillCircle(fb, 760, 290, 40, kSaffron);
    raster::circle(fb, 760, 290, 70, kCharcoal);

    Framebuffer small(23, 23);
    small.clear(kPaper);
    raster::circle(small, 11, 11, 10, kGreen);
    magnify(small, fb, 862, 6, 4);
    save(fb, "02-circles-ellipses-midpoint");
}

// 3. Polygons: scan-line fill of convex, concave and self-crossing shapes,
//    boundary fill inside a Bresenham outline, and flood fill.
void polygonsAndFill() {
    Framebuffer fb(960, 420);
    fb.clear(kPaper);

    const Polygon hexagon = polygon::regular({120, 120}, 90, 6, kPi / 6);
    render::fillPolygon(fb, hexagon, kGreen);
    render::drawPolygonOutline(fb, hexagon, kCharcoal);

    const Polygon star = polygon::star({340, 120}, 100, 42, 5, -kPi / 2);
    render::fillPolygon(fb, star, kSaffron);
    render::drawPolygonOutline(fb, star, kCharcoal);

    // A pentagram as one self-crossing outline: the even-odd rule leaves the
    // middle unfilled.
    Polygon pentagram;
    for (int i = 0; i < 5; ++i) {
        const float t = -kPi / 2 + 4.0f * kPi / 5.0f * static_cast<float>(i);
        pentagram.points.push_back({560 + 100 * std::cos(t), 125 + 100 * std::sin(t)});
    }
    render::fillPolygon(fb, pentagram, kTerracotta);
    render::drawPolygonOutline(fb, pentagram, kCharcoal);

    const Polygon arrow{{{700, 40}, {900, 120}, {700, 200}, {760, 120}}};
    render::fillPolygon(fb, arrow, kBlue);
    render::drawPolygonOutline(fb, arrow, kCharcoal);

    // Boundary fill: outline first, then fill from a seed inside it.
    const Polygon blob{{{60, 260}, {200, 240}, {260, 320}, {190, 400}, {120, 330}, {50, 380}}};
    render::drawPolygonOutline(fb, blob, kCharcoal);
    fill::boundaryFill(fb, 150, 300, kGreen, kCharcoal);
    raster::circle(fb, 400, 320, 70, kCharcoal);
    raster::circle(fb, 400, 320, 35, kCharcoal);
    fill::boundaryFill(fb, 400, 265, kSaffron, kCharcoal);
    fill::boundaryFill(fb, 400, 320, kTerracotta, kCharcoal);

    // Flood fill: recolour one connected region of a checkerboard.
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 8; ++i) {
            const Polygon cell{{{560.0f + i * 40, 250.0f + j * 40},
                                {600.0f + i * 40, 250.0f + j * 40},
                                {600.0f + i * 40, 290.0f + j * 40},
                                {560.0f + i * 40, 290.0f + j * 40}}};
            fill::polygon(fb, cell, (i + j) % 2 ? kSaffron : kPaper);
        }
    }
    fill::polygon(fb, Polygon{{{560, 330}, {880, 330}, {880, 370}, {560, 370}}}, kSaffron);
    fill::floodFill(fb, 570, 340, kTerracotta);
    save(fb, "03-polygons-scanline-boundary-flood-fill");
}

// 4. Clipping: Cohen-Sutherland lines and a Sutherland-Hodgman polygon
//    against the same window. Grey shows what was cut away.
void clipping() {
    Framebuffer fb(960, 420);
    fb.clear(kPaper);
    const int xmin = 80, ymin = 80, xmax = 400, ymax = 340;
    std::vector<std::array<int, 4>> segments;
    for (int i = 0; i < 24; ++i) {
        const float t = 2.0f * kPi * static_cast<float>(i) / 24.0f;
        const int x0 = 240 + static_cast<int>(200 * std::cos(t));
        const int y0 = 210 + static_cast<int>(190 * std::sin(t));
        const int x1 = 240 + static_cast<int>(220 * std::cos(t + 2.4f));
        const int y1 = 210 + static_cast<int>(200 * std::sin(t + 2.4f));
        raster::lineBresenham(fb, x0, y0, x1, y1, kGuide);
        segments.push_back({x0, y0, x1, y1});
    }
    for (auto s : segments) {
        if (raster::clipLine(s[0], s[1], s[2], s[3], xmin, ymin, xmax, ymax)) {
            raster::lineBresenham(fb, s[0], s[1], s[2], s[3], kTerracotta);
        }
    }
    frame(fb, xmin, ymin, xmax, ymax, kCharcoal);

    const raster::ClipRect window{560, 80, 880, 340};
    const Polygon star = polygon::star({720, 210}, 230, 110, 7, -kPi / 2);
    render::drawPolygonOutline(fb, star, kGuide);
    const Polygon clipped = raster::clipPolygon(star, window);
    fill::polygon(fb, clipped, kGreen);
    render::drawPolygonOutline(fb, clipped, kCharcoal);
    frame(fb, 560, 80, 880, 340, kCharcoal);
    save(fb, "04-clipping-cohen-sutherland-sutherland-hodgman");
}

// 5. Curves: a cubic Bezier with its control polygon and the de Casteljau
//    construction at t = 0.5, then Catmull-Rom and B-spline through the same
//    control points.
void curves() {
    Framebuffer fb(960, 420);
    fb.clear(kPaper);

    const std::vector<Vec2> c{{40, 360}, {110, 40}, {330, 40}, {400, 330}};
    render::drawPolygonOutline(fb, Polygon{{c[0], c[1], c[2], c[3], c[2], c[1]}}, kGuide);
    const Vec2 a = lerp(c[0], c[1], 0.5f), b = lerp(c[1], c[2], 0.5f), d = lerp(c[2], c[3], 0.5f);
    const Vec2 e = lerp(a, b, 0.5f), f = lerp(b, d, 0.5f);
    render::drawLine(fb, a, b, kSaffron);
    render::drawLine(fb, b, d, kSaffron);
    render::drawLine(fb, e, f, kTerracotta);
    const auto bez = bezier::sample(c, 64);
    for (std::size_t i = 0; i + 1 < bez.size(); ++i) {
        render::drawLine(fb, bez[i], bez[i + 1], kGreen);
    }
    for (Vec2 p : c) {
        dot(fb, p, kCharcoal);
    }
    dot(fb, lerp(e, f, 0.5f), kTerracotta);

    const std::vector<Vec2> pts{{500, 300}, {560, 90},  {660, 200}, {740, 60},
                                {860, 120}, {900, 330}, {720, 360}, {640, 280}};
    render::drawPolygonOutline(fb, Polygon{pts}, kGuide);
    const auto cr = spline::catmullRom(pts, 24, true);
    const auto bs = spline::bSpline(pts, 24, true);
    render::drawPolygonOutline(fb, Polygon{cr}, kBlue);
    render::drawPolygonOutline(fb, Polygon{bs}, kTerracotta);
    for (Vec2 p : pts) {
        dot(fb, p, kCharcoal);
    }
    save(fb, "05-curves-bezier-catmull-rom-b-spline");
}

// 6. The six motifs, each built only from the primitives above.
void motifs() {
    Framebuffer fb(960, 340);
    fb.clear(kPaper);
    const auto& all = builtinMotifs();
    for (std::size_t i = 0; i < all.size(); ++i) {
        const float cx = 90.0f + 156.0f * static_cast<float>(i);
        render::drawMotif(fb, all[i], Mat3::translate(cx, 90) * Mat3::scale(140, 140),
                          kDefaults.palette);
        // Second row: the same motif as outline only, drawn with DDA.
        render::RenderOptions outlineOnly;
        outlineOnly.fill = false;
        outlineOnly.lines = render::LineAlgorithm::DDA;
        render::drawMotif(fb, all[i], Mat3::translate(cx, 250) * Mat3::scale(140, 140),
                          kDefaults.palette, outlineOnly);
    }
    save(fb, "06-motifs-built-from-primitives");
}

// 7. Patterns: each motif tiled across a canvas, partial tiles clipped.
void patterns() {
    Framebuffer sheet(960, 640);
    sheet.clear(kCharcoal);
    for (int i = 0; i < 6; ++i) {
        PatternSettings s;
        s.motifIndex = i;
        s.tileSize = 80;
        s.spacing = 6;
        Framebuffer tile(312, 312);
        render::drawPattern(tile, s);
        const int ox = 6 + (i % 3) * 318;
        const int oy = 6 + (i / 3) * 318;
        for (int y = 0; y < tile.height(); ++y) {
            for (int x = 0; x < tile.width(); ++x) {
                sheet.setPixel(ox + x, oy + y, tile.getPixel(x, y));
            }
        }
    }
    save(sheet, "07-patterns-tiled-and-clipped");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1) {
        outDir = argv[1];
    }
    std::error_code ec;
    std::filesystem::create_directories(outDir, ec);
    lines();
    circlesAndEllipses();
    polygonsAndFill();
    clipping();
    curves();
    motifs();
    patterns();
    return 0;
}
