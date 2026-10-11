#include "motifs/MotifLibrary.h"

#include <utility>

#include "geometry/PolygonOps.h"

namespace kitenge {

namespace {

Shape filled(Primitive geometry, int fill, int stroke = kInk) {
    return Shape{std::move(geometry), fill, stroke};
}

Shape stroked(Primitive geometry, int stroke) {
    return Shape{std::move(geometry), kNoColor, stroke};
}

// The four tile corners, cut as small triangles. Neighbouring tiles complete
// them into diamonds where four tiles meet.
void addCornerTriangles(Motif& m, float size, int fill) {
    for (float sx : {-1.0f, 1.0f}) {
        for (float sy : {-1.0f, 1.0f}) {
            const Vec2 corner{0.5f * sx, 0.5f * sy};
            m.shapes.push_back(filled(
                Polygon{
                    {corner, {corner.x - size * sx, corner.y}, {corner.x, corner.y - size * sy}}},
                fill, kNoColor));
        }
    }
}

Motif diamond() {
    Motif m{"Diamond", {}};
    addCornerTriangles(m, 0.2f, kPrimary);
    m.shapes.push_back(filled(Polygon{{{0, -0.45f}, {0.45f, 0}, {0, 0.45f}, {-0.45f, 0}}}, kWarm));
    m.shapes.push_back(
        filled(Polygon{{{0, -0.25f}, {0.25f, 0}, {0, 0.25f}, {-0.25f, 0}}}, kAccent));
    m.shapes.push_back(filled(Circle{{0, 0}, 0.07f}, kInk));
    return m;
}

Motif triangle() {
    Motif m{"Triangle", {}};
    const Vec2 top{0, -0.42f}, right{0.42f, 0.34f}, left{-0.42f, 0.34f};
    m.shapes.push_back(filled(Polygon{{top, right, left}}, kAccent));
    // The medial triangle, pointing down, splits it into the classic four.
    m.shapes.push_back(filled(
        Polygon{{lerp(top, left, 0.5f), lerp(top, right, 0.5f), lerp(left, right, 0.5f)}}, kWarm));
    m.shapes.push_back(stroked(Segment{{-0.42f, 0.43f}, {0.42f, 0.43f}}, kPrimary));
    m.shapes.push_back(stroked(Segment{{-0.42f, -0.43f}, {-0.2f, -0.43f}}, kPrimary));
    m.shapes.push_back(stroked(Segment{{0.2f, -0.43f}, {0.42f, -0.43f}}, kPrimary));
    return m;
}

Motif cross() {
    Motif m{"Cross", {}};
    const float w = 0.1f;
    const float l = 0.42f;
    m.shapes.push_back(filled(Polygon{{{-w, -l},
                                       {w, -l},
                                       {w, -w},
                                       {l, -w},
                                       {l, w},
                                       {w, w},
                                       {w, l},
                                       {-w, l},
                                       {-w, w},
                                       {-l, w},
                                       {-l, -w},
                                       {-w, -w}}},
                              kPrimary));
    m.shapes.push_back(filled(polygon::regular({0, 0}, 0.13f, 4), kAccent));
    for (float sx : {-1.0f, 1.0f}) {
        for (float sy : {-1.0f, 1.0f}) {
            m.shapes.push_back(
                stroked(Segment{{0.2f * sx, 0.2f * sy}, {0.42f * sx, 0.42f * sy}}, kWarm));
        }
    }
    return m;
}

}  // namespace

const std::vector<Motif>& builtinMotifs() {
    static const std::vector<Motif> motifs = {diamond(), triangle(), cross()};
    return motifs;
}

}  // namespace kitenge
