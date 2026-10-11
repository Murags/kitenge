#pragma once

#include <vector>

#include "core/Color.h"
#include "symmetry/Symmetry.h"

namespace kitenge {

// Everything the control panel lets the user change. The UI edits it, and
// the pattern is redrawn from it whenever it changes.
struct PatternSettings {
    int motifIndex = 0;
    SymmetryRule rule = SymmetryRule::FourFold;
    int tileSize = 125;  // pixels
    int spacing = 12;    // pixels between tiles

    // Default palette from the proposal: green, saffron, terracotta, charcoal.
    std::vector<Color> palette = {
        Color::fromHex(0x1F6F57),
        Color::fromHex(0xF0A030),
        Color::fromHex(0xD95B33),
        Color::fromHex(0x2B2B2B),
    };
    Color background = Color::fromHex(0xFAEBD7);

    bool operator==(const PatternSettings& o) const {
        return motifIndex == o.motifIndex && rule == o.rule && tileSize == o.tileSize &&
               spacing == o.spacing && palette == o.palette && background == o.background;
    }
    bool operator!=(const PatternSettings& o) const { return !(*this == o); }
};

}  // namespace kitenge
