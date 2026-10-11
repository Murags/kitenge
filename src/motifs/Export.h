#pragma once

#include <string>

#include "core/Framebuffer.h"

namespace kitenge {

// Writes the framebuffer as an uncompressed 24-bit BMP. The encoder is
// written by hand (header fields, bottom-up rows padded to 4 bytes), so
// exporting needs no image library. Alpha is dropped. Returns false if the
// file cannot be written.
bool writeBmp(const Framebuffer& fb, const std::string& path);

}  // namespace kitenge
