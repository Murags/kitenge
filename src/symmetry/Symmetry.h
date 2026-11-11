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
// Each copy's transform is translate(cell centre) * scale(tileSize) * cell
// transform, where the cell transform comes from the rule: identity for
// Translate, a quarter turn per cell round each 2x2 block for 4-fold, and a
// reflection on odd columns and/or rows for Mirror. Parity is counted from the
// centre cell, so the middle tile always shows the motif unchanged.
std::vector<TilePlacement> layoutTiles(SymmetryRule rule, int tileSize, int spacing,
                                       int canvasWidth, int canvasHeight);

}  // namespace kitenge
