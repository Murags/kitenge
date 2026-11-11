#include "render/PatternRenderer.h"

#include <algorithm>

#include "motifs/MotifLibrary.h"
#include "symmetry/Symmetry.h"

namespace kitenge::render {

int drawPattern(Framebuffer& fb, const PatternSettings& settings, const RenderOptions& options,
                int maxTiles) {
    fb.clear(settings.background);
    const auto& motifs = builtinMotifs();
    if (motifs.empty()) {
        return 0;
    }
    const int index = std::clamp(settings.motifIndex, 0, static_cast<int>(motifs.size()) - 1);
    const auto tiles =
        layoutTiles(settings.rule, settings.tileSize, settings.spacing, fb.width(), fb.height());
    const int total = static_cast<int>(tiles.size());
    const int count = maxTiles < 0 ? total : std::min(maxTiles, total);
    for (int i = 0; i < count; ++i) {
        drawMotif(fb, motifs[static_cast<std::size_t>(index)],
                  tiles[static_cast<std::size_t>(i)].transform, settings.palette, options);
    }
    return count;
}

int tileCount(const PatternSettings& settings, int canvasWidth, int canvasHeight) {
    return static_cast<int>(
        layoutTiles(settings.rule, settings.tileSize, settings.spacing, canvasWidth, canvasHeight)
            .size());
}

}  // namespace kitenge::render
