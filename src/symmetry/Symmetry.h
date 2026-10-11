#pragma once

#include <vector>

#include "symmetry/Mat3.h"

namespace kitenge {

// The three symmetry rules in scope for the project.
enum class SymmetryRule {
    Translate,  // every copy is the motif shifted to its tile
    FourFold,   // copies are rotated by 0, 90, 180 and 270 degrees in turn
    Mirror,     // neighbouring copies are reflected across the tile edge
};

const char* toString(SymmetryRule rule);

// One copy of the motif: the transform from tile-local motif coordinates to
// canvas pixels, plus its grid cell. `order` is the position in the animated
// assembly, so playback can reveal copies one at a time.
struct TilePlacement {
    Mat3 transform;
    int row = 0;
    int col = 0;
    int order = 0;
};

// Lays copies of a motif across a canvas of the given size. Tiles are
// `tileSize` pixels square with `spacing` pixels between them; the grid is
// centred on the canvas and runs past its edges so the pattern has no border
// (the renderer clips the partial tiles).
//
// Checkpoint 1 places every copy with a window-to-viewport transform (scale
// the unit tile to `tileSize`, translate it to its cell), whatever the rule.
// The per-cell rotations and reflections of the 4-fold and Mirror rules are
// added in Checkpoint 2.
std::vector<TilePlacement> layoutTiles(SymmetryRule rule, int tileSize, int spacing,
                                       int canvasWidth, int canvasHeight);

}  // namespace kitenge
