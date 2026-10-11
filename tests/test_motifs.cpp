#include <doctest/doctest.h>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

#include "motifs/Export.h"
#include "motifs/MotifLibrary.h"

using namespace kitenge;

namespace {

bool insideTile(Vec2 p) {
    return p.x >= -0.5f && p.x <= 0.5f && p.y >= -0.5f && p.y <= 0.5f;
}

// Every point (and circle/ellipse extent) of a primitive stays in the tile.
bool primitiveInTile(const Primitive& prim) {
    return std::visit(
        [](const auto& g) {
            using T = std::decay_t<decltype(g)>;
            if constexpr (std::is_same_v<T, Segment>) {
                return insideTile(g.a) && insideTile(g.b);
            } else if constexpr (std::is_same_v<T, Polygon>) {
                for (Vec2 p : g.points) {
                    if (!insideTile(p)) {
                        return false;
                    }
                }
                return g.points.size() >= 3;
            } else if constexpr (std::is_same_v<T, Circle>) {
                return insideTile(g.centre + Vec2(g.radius, g.radius)) &&
                       insideTile(g.centre - Vec2(g.radius, g.radius));
            } else if constexpr (std::is_same_v<T, Ellipse>) {
                return insideTile(g.centre + Vec2(g.rx, g.ry)) &&
                       insideTile(g.centre - Vec2(g.rx, g.ry));
            } else if constexpr (std::is_same_v<T, BezierPath>) {
                return insideTile(g.start) && !g.segments.empty() &&
                       g.segments.back().end == g.start;
            } else {
                for (Vec2 p : g.points) {
                    if (!insideTile(p)) {
                        return false;
                    }
                }
                return g.points.size() >= 3;
            }
        },
        prim);
}

std::uint32_t read32(const std::vector<unsigned char>& b, std::size_t at) {
    return b[at] | (b[at + 1] << 8) | (b[at + 2] << 16) |
           (static_cast<std::uint32_t>(b[at + 3]) << 24);
}

}  // namespace

TEST_CASE("the library has the six motifs in control panel order") {
    const auto& motifs = builtinMotifs();
    REQUIRE(motifs.size() == 6u);
    const char* names[] = {"Leaf", "Diamond", "Triangle", "Cross", "Ring", "Bow"};
    for (std::size_t i = 0; i < motifs.size(); ++i) {
        CHECK(motifs[i].name == names[i]);
    }
}

TEST_CASE("every motif shape fits in the tile and uses valid palette slots") {
    for (const Motif& m : builtinMotifs()) {
        CAPTURE(m.name);
        CHECK_FALSE(m.shapes.empty());
        for (const Shape& s : m.shapes) {
            CHECK(primitiveInTile(s.geometry));
            CHECK(s.fillIndex >= kNoColor);
            CHECK(s.fillIndex <= kInk);
            CHECK(s.strokeIndex >= kNoColor);
            CHECK(s.strokeIndex <= kInk);
            CHECK((s.fillIndex != kNoColor || s.strokeIndex != kNoColor));
        }
    }
}

TEST_CASE("BMP export writes a valid header and bottom-up padded rows") {
    Framebuffer fb(3, 2);
    fb.clear(Color(10, 20, 30));
    fb.setPixel(0, 0, Color(255, 0, 0));  // top-left
    const std::string path = "kitenge_test_export.bmp";
    REQUIRE(writeBmp(fb, path));

    std::ifstream in(path, std::ios::binary);
    const std::vector<unsigned char> b((std::istreambuf_iterator<char>(in)),
                                       std::istreambuf_iterator<char>());
    in.close();
    std::remove(path.c_str());

    const std::size_t rowBytes = 12;  // 3 pixels * 3 bytes = 9, padded to 12
    REQUIRE(b.size() == 54 + rowBytes * 2);
    CHECK(b[0] == 'B');
    CHECK(b[1] == 'M');
    CHECK(read32(b, 2) == b.size());
    CHECK(read32(b, 10) == 54u);
    CHECK(read32(b, 18) == 3u);
    CHECK(read32(b, 22) == 2u);

    // The top row is stored last, as blue, green, red.
    const std::size_t top = 54 + rowBytes;
    CHECK(b[top + 0] == 0);
    CHECK(b[top + 1] == 0);
    CHECK(b[top + 2] == 255);
    CHECK(b[54 + 0] == 30);
    CHECK(b[54 + 1] == 20);
    CHECK(b[54 + 2] == 10);
}
