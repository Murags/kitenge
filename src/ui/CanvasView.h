#pragma once

#include <SDL.h>

#include "core/Framebuffer.h"

namespace kitenge {

// Shows our Framebuffer on screen. The pixels are copied unchanged into an
// SDL texture; SDL never draws anything onto the canvas itself.
class CanvasView {
public:
    explicit CanvasView(SDL_Renderer* renderer) : renderer_(renderer) {}
    ~CanvasView();

    CanvasView(const CanvasView&) = delete;
    CanvasView& operator=(const CanvasView&) = delete;

    // Copies the framebuffer into the texture, recreating the texture when
    // the size has changed.
    void upload(const Framebuffer& fb);

    // Null until the first non-empty upload.
    SDL_Texture* texture() const { return texture_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    SDL_Renderer* renderer_;
    SDL_Texture* texture_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace kitenge
