#include <doctest/doctest.h>

#include <map>
#include <utility>

#include "symmetry/Symmetry.h"

using namespace kitenge;

namespace {

constexpr int kTile = 100;

// Tiles of a 600 x 400 canvas keyed by their offset from the centre cell.
std::map<std::pair<int, int>, TilePlacement> byOffset(SymmetryRule rule) {
    const auto tiles = layoutTiles(rule, kTile, 20, 600, 400);
    const TilePlacement* centre = nullptr;
    for (const TilePlacement& t : tiles) {
        const Vec2 c = t.transform.apply({0, 0});
        if (c.x == doctest::Approx(300.0f) && c.y == doctest::Approx(200.0f)) {
            centre = &t;
        }
    }
    REQUIRE(centre != nullptr);
    std::map<std::pair<int, int>, TilePlacement> out;
    for (const TilePlacement& t : tiles) {
        out[{t.row - centre->row, t.col - centre->col}] = t;
    }
    return out;
}

// Where the motif's local +x and +y axes point on the canvas, in tile units.
void checkAxes(const TilePlacement& t, Vec2 x, Vec2 y) {
    const Vec2 o = t.transform.apply({0, 0});
    const Vec2 ax = (t.transform.apply({1, 0}) - o) / static_cast<float>(kTile);
    const Vec2 ay = (t.transform.apply({0, 1}) - o) / static_cast<float>(kTile);
    CHECK(ax.x == doctest::Approx(x.x).epsilon(1e-4));
    CHECK(ax.y == doctest::Approx(x.y).epsilon(1e-4));
    CHECK(ay.x == doctest::Approx(y.x).epsilon(1e-4));
    CHECK(ay.y == doctest::Approx(y.y).epsilon(1e-4));
}

}  // namespace

TEST_CASE("Translate keeps every copy upright") {
    for (const auto& entry : byOffset(SymmetryRule::Translate)) {
        checkAxes(entry.second, {1, 0}, {0, 1});
    }
}

TEST_CASE("4-fold turns each copy a quarter more going clockwise round a 2x2 block") {
    const auto tiles = byOffset(SymmetryRule::FourFold);
    checkAxes(tiles.at({0, 0}), {1, 0}, {0, 1});    // 0 degrees
    checkAxes(tiles.at({0, 1}), {0, 1}, {-1, 0});   // 90
    checkAxes(tiles.at({1, 1}), {-1, 0}, {0, -1});  // 180
    checkAxes(tiles.at({1, 0}), {0, -1}, {1, 0});   // 270
    // The block repeats every two cells in both directions.
    checkAxes(tiles.at({2, 2}), {1, 0}, {0, 1});
    checkAxes(tiles.at({-1, -1}), {-1, 0}, {0, -1});
}

TEST_CASE("Mirror reflects neighbours across the edge they share") {
    const auto tiles = byOffset(SymmetryRule::Mirror);
    checkAxes(tiles.at({0, 0}), {1, 0}, {0, 1});
    checkAxes(tiles.at({0, 1}), {-1, 0}, {0, 1});  // left-right flip
    checkAxes(tiles.at({0, -1}), {-1, 0}, {0, 1});
    checkAxes(tiles.at({1, 0}), {1, 0}, {0, -1});   // top-bottom flip
    checkAxes(tiles.at({1, 1}), {-1, 0}, {0, -1});  // both
}

TEST_CASE("every rule places copies on the same grid of cell centres") {
    const auto translate = byOffset(SymmetryRule::Translate);
    for (SymmetryRule rule : {SymmetryRule::FourFold, SymmetryRule::Mirror}) {
        const auto tiles = byOffset(rule);
        REQUIRE(tiles.size() == translate.size());
        for (const auto& [offset, tile] : tiles) {
            const Vec2 a = tile.transform.apply({0, 0});
            const Vec2 b = translate.at(offset).transform.apply({0, 0});
            CHECK(a.x == doctest::Approx(b.x));
            CHECK(a.y == doctest::Approx(b.y));
        }
    }
}
