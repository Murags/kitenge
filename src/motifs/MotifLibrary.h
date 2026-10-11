#pragma once

#include <vector>

#include "geometry/Motif.h"

namespace kitenge {

// Palette slots the built-in motifs refer to. With the default palette these
// are green, saffron, terracotta and charcoal.
enum PaletteSlot : int {
    kPrimary = 0,
    kAccent = 1,
    kWarm = 2,
    kInk = 3,
};

// The six built-in motifs, in the order the control panel lists them:
// Leaf, Diamond, Triangle, Cross, Ring, Bow. Each is built only from the
// primitives in Motif.h, inside the tile square (-0.5, -0.5) to (0.5, 0.5).
const std::vector<Motif>& builtinMotifs();

}  // namespace kitenge
