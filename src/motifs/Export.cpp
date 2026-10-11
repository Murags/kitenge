#include "motifs/Export.h"

#include <cstdint>
#include <fstream>
#include <vector>

namespace kitenge {

namespace {

void put16(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>(v));
    out.push_back(static_cast<std::uint8_t>(v >> 8));
}

void put32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    for (int shift = 0; shift < 32; shift += 8) {
        out.push_back(static_cast<std::uint8_t>(v >> shift));
    }
}

}  // namespace

bool writeBmp(const Framebuffer& fb, const std::string& path) {
    const std::uint32_t width = static_cast<std::uint32_t>(fb.width());
    const std::uint32_t height = static_cast<std::uint32_t>(fb.height());
    const std::uint32_t rowBytes = (width * 3 + 3) & ~3u;  // rows pad to 4 bytes
    const std::uint32_t pixelBytes = rowBytes * height;
    const std::uint32_t headerBytes = 14 + 40;

    std::vector<std::uint8_t> out;
    out.reserve(headerBytes + pixelBytes);

    // BITMAPFILEHEADER
    out.push_back('B');
    out.push_back('M');
    put32(out, headerBytes + pixelBytes);
    put32(out, 0);  // reserved
    put32(out, headerBytes);

    // BITMAPINFOHEADER
    put32(out, 40);
    put32(out, width);
    put32(out, height);  // positive height: rows stored bottom-up
    put16(out, 1);       // colour planes
    put16(out, 24);      // bits per pixel
    put32(out, 0);       // no compression
    put32(out, pixelBytes);
    put32(out, 2835);  // 72 DPI in pixels per metre
    put32(out, 2835);
    put32(out, 0);  // palette size
    put32(out, 0);

    for (int y = fb.height() - 1; y >= 0; --y) {
        for (int x = 0; x < fb.width(); ++x) {
            const Color c = fb.getPixel(x, y);
            out.push_back(c.b);
            out.push_back(c.g);
            out.push_back(c.r);
        }
        for (std::uint32_t pad = width * 3; pad < rowBytes; ++pad) {
            out.push_back(0);
        }
    }

    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
    return static_cast<bool>(file);
}

}  // namespace kitenge
