#pragma once

#include "core/PatternSettings.h"
#include "ui/CanvasView.h"

namespace kitenge {

// Buttons in the top bar that were clicked this frame.
struct StudioActions {
    bool animate = false;
    bool preview3d = false;
    bool exportImage = false;
};

// Figures reported in the status line under the canvas.
struct StudioStatus {
    int tilesDrawn = 0;
};

struct StudioFrame {
    // Size the canvas area has on screen this frame. The framebuffer should be
    // resized to match before the next frame.
    int canvasWidth = 0;
    int canvasHeight = 0;
    StudioActions actions;
};

// Draws the whole studio window, as in the proposal mock-up: top bar,
// controls on the left, the pattern canvas on the right and a status line
// underneath. Edits `settings` in place.
StudioFrame drawStudio(PatternSettings& settings, const CanvasView& canvas,
                       const StudioStatus& status);

}  // namespace kitenge
