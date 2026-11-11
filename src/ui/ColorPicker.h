#pragma once

#include <SDL.h>

#include "core/Color.h"
#include "core/ColorModel.h"
#include "core/Framebuffer.h"
#include "ui/CanvasView.h"

namespace kitenge {

// Our own RGB / HSV colour picker. The saturation-value square and the hue
// bar are rasterised pixel by pixel into Framebuffers from the HSV model and
// shown through CanvasView; ImGui only supplies the mouse input and sliders.
class ColorPicker {
public:
    explicit ColorPicker(SDL_Renderer* renderer);

    // Draws the picker for `color` and returns true when the user changed it.
    bool draw(Color& color);

    // Which colour the picker is editing: a palette slot, or the background
    // when it equals the palette size.
    int target = 0;

private:
    void rasteriseSquare(float hue);
    void rasteriseHueBar();

    Framebuffer square_;
    Framebuffer hueBar_;
    CanvasView squareView_;
    CanvasView hueView_;
    float squareHue_ = -1.0f;

    // Kept between frames so the hue survives while saturation or value is 0.
    Hsv hsv_;
    Color last_;
    bool hasLast_ = false;
};

}  // namespace kitenge
