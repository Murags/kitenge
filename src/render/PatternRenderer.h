#pragma once

#include "core/Framebuffer.h"
#include "core/PatternSettings.h"
#include "render/MotifRenderer.h"

namespace kitenge::render {

// Clears the framebuffer to the background colour and draws the selected
// built-in motif into every tile of the layout. Returns how many tiles were
// drawn.
int drawPattern(Framebuffer& fb, const PatternSettings& settings,
                const RenderOptions& options = {});

}  // namespace kitenge::render
