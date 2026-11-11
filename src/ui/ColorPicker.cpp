#include "ui/ColorPicker.h"

#include <imgui.h>

#include <algorithm>
#include <cstdint>

namespace kitenge {

namespace {

constexpr int kSquareSize = 160;
constexpr int kHueWidth = 18;

ImTextureID textureId(const CanvasView& view) {
    return static_cast<ImTextureID>(reinterpret_cast<std::intptr_t>(view.texture()));
}

// An invisible button over the image just drawn; while it is held, returns
// the mouse position as a fraction (0..1) of the image in each axis.
bool dragFraction(const char* id, ImVec2 origin, ImVec2 size, float& fx, float& fy) {
    ImGui::SetCursorScreenPos(origin);
    ImGui::InvisibleButton(id, size);
    if (!ImGui::IsItemActive()) {
        return false;
    }
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    fx = std::clamp((mouse.x - origin.x) / size.x, 0.0f, 1.0f);
    fy = std::clamp((mouse.y - origin.y) / size.y, 0.0f, 1.0f);
    return true;
}

}  // namespace

ColorPicker::ColorPicker(SDL_Renderer* renderer)
    : square_(kSquareSize, kSquareSize),
      hueBar_(kHueWidth, kSquareSize),
      squareView_(renderer),
      hueView_(renderer) {
    rasteriseHueBar();
}

void ColorPicker::rasteriseSquare(float hue) {
    // Saturation grows left to right, value falls top to bottom.
    for (int y = 0; y < square_.height(); ++y) {
        const float v = 1.0f - static_cast<float>(y) / static_cast<float>(square_.height() - 1);
        for (int x = 0; x < square_.width(); ++x) {
            const float s = static_cast<float>(x) / static_cast<float>(square_.width() - 1);
            square_.setPixel(x, y, toRgb({hue, s, v}));
        }
    }
    squareView_.upload(square_);
    squareHue_ = hue;
}

void ColorPicker::rasteriseHueBar() {
    for (int y = 0; y < hueBar_.height(); ++y) {
        const float h = 360.0f * static_cast<float>(y) / static_cast<float>(hueBar_.height());
        const Color c = toRgb({h, 1.0f, 1.0f});
        for (int x = 0; x < hueBar_.width(); ++x) {
            hueBar_.setPixel(x, y, c);
        }
    }
    hueView_.upload(hueBar_);
}

bool ColorPicker::draw(Color& color) {
    if (!hasLast_ || color != last_) {
        hsv_ = toHsv(color);
        last_ = color;
        hasLast_ = true;
    }
    if (hsv_.h != squareHue_) {
        rasteriseSquare(hsv_.h);
    }

    bool changed = false;
    const ImVec2 squareSize(static_cast<float>(kSquareSize), static_cast<float>(kSquareSize));
    const ImVec2 hueSize(static_cast<float>(kHueWidth), static_cast<float>(kSquareSize));
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 hueOrigin(origin.x + squareSize.x + ImGui::GetStyle().ItemSpacing.x, origin.y);

    // The images first, then invisible buttons over them for the mouse.
    ImGui::Image(textureId(squareView_), squareSize);
    ImGui::SetCursorScreenPos(hueOrigin);
    ImGui::Image(textureId(hueView_), hueSize);

    float fx = 0.0f, fy = 0.0f;
    if (dragFraction("##sv", origin, squareSize, fx, fy)) {
        hsv_.s = fx;
        hsv_.v = 1.0f - fy;
        changed = true;
    }
    if (dragFraction("##hue", hueOrigin, hueSize, fx, fy)) {
        hsv_.h = std::min(fy * 360.0f, 359.9f);
        changed = true;
    }

    // Markers for the current position on the square and the bar.
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 mark(origin.x + hsv_.s * squareSize.x, origin.y + (1.0f - hsv_.v) * squareSize.y);
    draw->AddCircle(mark, 5.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    draw->AddCircle(mark, 6.5f, IM_COL32(0, 0, 0, 255));
    const float hueY = hueOrigin.y + hsv_.h / 360.0f * hueSize.y;
    draw->AddRect(ImVec2(hueOrigin.x - 2, hueY - 2), ImVec2(hueOrigin.x + hueSize.x + 2, hueY + 2),
                  IM_COL32(0, 0, 0, 255), 0.0f, 0, 2.0f);

    ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + squareSize.y));
    ImGui::Spacing();

    // Exact values in both models.
    const float third =
        (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3;
    float s = hsv_.s * 100.0f;
    float v = hsv_.v * 100.0f;
    ImGui::SetNextItemWidth(third);
    changed |= ImGui::SliderFloat("##h", &hsv_.h, 0.0f, 359.9f, "H %.0f");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(third);
    if (ImGui::SliderFloat("##s", &s, 0.0f, 100.0f, "S %.0f%%")) {
        hsv_.s = s / 100.0f;
        changed = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(third);
    if (ImGui::SliderFloat("##v", &v, 0.0f, 100.0f, "V %.0f%%")) {
        hsv_.v = v / 100.0f;
        changed = true;
    }

    Color rgb = changed ? toRgb(hsv_, color.a) : color;
    int channels[3] = {rgb.r, rgb.g, rgb.b};
    const char* formats[3] = {"R %d", "G %d", "B %d"};
    bool rgbChanged = false;
    for (int i = 0; i < 3; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        ImGui::PushID(i);
        ImGui::SetNextItemWidth(third);
        rgbChanged |= ImGui::SliderInt("##rgb", &channels[i], 0, 255, formats[i]);
        ImGui::PopID();
    }
    if (rgbChanged) {
        rgb = Color(static_cast<std::uint8_t>(channels[0]), static_cast<std::uint8_t>(channels[1]),
                    static_cast<std::uint8_t>(channels[2]), color.a);
        hsv_ = toHsv(rgb);
        changed = true;
    }

    if (changed) {
        color = rgb;
        last_ = color;
    }
    return changed;
}

}  // namespace kitenge
