#include "symmetry/Symmetry.h"

#include <algorithm>

namespace kitenge {

const char* toString(SymmetryRule rule) {
    switch (rule) {
        case SymmetryRule::Translate:
            return "Translate";
        case SymmetryRule::FourFold:
            return "4-fold";
        case SymmetryRule::Mirror:
            return "Mirror";
    }
    return "Unknown";
}

std::vector<TilePlacement> layoutTiles(SymmetryRule /*rule*/, int tileSize, int spacing,
                                       int canvasWidth, int canvasHeight) {
    std::vector<TilePlacement> tiles;
    if (tileSize <= 0 || canvasWidth <= 0 || canvasHeight <= 0) {
        return tiles;
    }
    spacing = std::max(spacing, 0);
    const int pitch = tileSize + spacing;

    // Enough cells to cover the canvas once the grid is centred, plus one on
    // each side for the partial tiles at the edges. Odd counts keep a tile
    // exactly in the middle.
    auto cellsFor = [pitch](int extent) { return (extent / pitch + 2) | 1; };
    const int cols = cellsFor(canvasWidth);
    const int rows = cellsFor(canvasHeight);
    const float originX = canvasWidth / 2.0f - (cols / 2) * static_cast<float>(pitch);
    const float originY = canvasHeight / 2.0f - (rows / 2) * static_cast<float>(pitch);
    const float halfTile = tileSize / 2.0f;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const float cx = originX + col * static_cast<float>(pitch);
            const float cy = originY + row * static_cast<float>(pitch);
            // Skip cells that would not touch the canvas at all.
            if (cx + halfTile < 0 || cy + halfTile < 0 || cx - halfTile > canvasWidth ||
                cy - halfTile > canvasHeight) {
                continue;
            }
            TilePlacement t;
            t.transform = Mat3::translate(cx, cy) *
                          Mat3::scale(static_cast<float>(tileSize), static_cast<float>(tileSize));
            t.row = row;
            t.col = col;
            t.order = static_cast<int>(tiles.size());
            tiles.push_back(t);
        }
    }
    return tiles;
}

}  // namespace kitenge
