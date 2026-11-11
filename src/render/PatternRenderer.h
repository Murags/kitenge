#pragma once

#include "core/Framebuffer.h"
#include "core/PatternSettings.h"
#include "render/MotifRenderer.h"

namespace kitenge::render {

// Clears the framebuffer to the background colour and draws the selected
// built-in motif into the tiles of the layout, in assembly order. With
// `maxTiles` >= 0 only that many tiles are drawn, which is how the animated
// assembly reveals the pattern a copy at a time. Returns how many tiles were
// drawn.
int drawPattern(Framebuffer& fb, const PatternSettings& settings, const RenderOptions& options = {},
                int maxTiles = -1);

// How many tiles the full pattern has for this canvas size.
int tileCount(const PatternSettings& settings, int canvasWidth, int canvasHeight);

}  // namespace kitenge::render
