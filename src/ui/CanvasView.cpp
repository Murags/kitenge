#include "ui/CanvasView.h"

#include <cstdio>

namespace kitenge {

CanvasView::~CanvasView() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
}

void CanvasView::upload(const Framebuffer& fb) {
    if (fb.width() == 0 || fb.height() == 0) {
        return;
    }

    if (!texture_ || fb.width() != width_ || fb.height() != height_) {
        if (texture_) {
            SDL_DestroyTexture(texture_);
        }
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                     fb.width(), fb.height());
        if (!texture_) {
            std::fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
            width_ = height_ = 0;
            return;
        }
        // Keep pixels sharp when the canvas is scaled on high-DPI screens.
        SDL_SetTextureScaleMode(texture_, SDL_ScaleModeNearest);
        width_ = fb.width();
        height_ = fb.height();
    }

    SDL_UpdateTexture(texture_, nullptr, fb.data(), fb.pitchBytes());
}

}  // namespace kitenge
