#pragma once

#include <utility>

#include "core/Framebuffer.h"

namespace kitenge {

// Two framebuffers of the same size. Everything is drawn into the back
// buffer, which is never shown; swap() then makes the finished frame the
// front buffer in one step. The screen only ever sees complete frames, so
// redraws and the animated assembly do not flicker.
class DoubleBuffer {
public:
    int width() const { return front_.width(); }
    int height() const { return front_.height(); }

    // Resizes both buffers, discarding their contents.
    void resize(int width, int height) {
        front_.resize(width, height);
        back_.resize(width, height);
    }

    Framebuffer& back() { return back_; }
    const Framebuffer& front() const { return front_; }

    // Shows the frame just drawn into the back buffer.
    void swap() { std::swap(front_, back_); }

private:
    Framebuffer front_;
    Framebuffer back_;
};

}  // namespace kitenge
