#include "render/PatternRenderer.h"

#include <algorithm>

#include "motifs/MotifLibrary.h"
#include "symmetry/Symmetry.h"

namespace kitenge::render {

int drawPattern(Framebuffer& fb, const PatternSettings& settings, const RenderOptions& options) {
    fb.clear(settings.background);
    const auto& motifs = builtinMotifs();
    if (motifs.empty()) {
        return 0;
    }
    const int index = std::clamp(settings.motifIndex, 0, static_cast<int>(motifs.size()) - 1);
    const auto tiles =
        layoutTiles(settings.rule, settings.tileSize, settings.spacing, fb.width(), fb.height());
    for (const TilePlacement& tile : tiles) {
        drawMotif(fb, motifs[static_cast<std::size_t>(index)], tile.transform, settings.palette,
                  options);
    }
    return static_cast<int>(tiles.size());
}

}  // namespace kitenge::render
