#include "ui/StudioLayout.h"

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <string>

namespace kitenge {

namespace {

// Placeholder names until the motif library provides the six built-in motifs.
constexpr const char* kMotifNames[] = {"Leaf", "Diamond", "Triangle", "Cross", "Ring", "Bow"};
constexpr int kMotifCount = sizeof(kMotifNames) / sizeof(kMotifNames[0]);

constexpr SymmetryRule kRules[] = {SymmetryRule::FourFold, SymmetryRule::Mirror,
                                   SymmetryRule::Translate};

constexpr float kControlsWidth = 300.0f;

ImVec4 toImVec4(Color c) {
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

Color fromImVec4(const ImVec4& v) {
    auto channel = [](float f) {
        return static_cast<std::uint8_t>(std::clamp(f, 0.0f, 1.0f) * 255.0f + 0.5f);
    };
    return Color(channel(v.x), channel(v.y), channel(v.z), channel(v.w));
}

void sectionLabel(const char* text) {
    ImGui::Spacing();
    ImGui::TextDisabled("%s", text);
}

StudioActions drawTopBar() {
    StudioActions actions;
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Kitenge - pattern studio");

    const ImGuiStyle& style = ImGui::GetStyle();
    const char* labels[] = {"Animate", "3D preview", "Export"};
    float buttonsWidth = 0.0f;
    for (const char* label : labels) {
        buttonsWidth += ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    }
    buttonsWidth += style.ItemSpacing.x * 2.0f;
    ImGui::SameLine(ImGui::GetContentRegionMax().x - buttonsWidth);

    // Wired up as the matching features land (animation, OpenGL, export).
    ImGui::BeginDisabled();
    actions.animate = ImGui::Button(labels[0]);
    ImGui::SameLine();
    actions.preview3d = ImGui::Button(labels[1]);
    ImGui::SameLine();
    actions.exportImage = ImGui::Button(labels[2]);
    ImGui::EndDisabled();

    ImGui::Separator();
    return actions;
}

void drawControls(PatternSettings& settings) {
    sectionLabel("Motif");
    const float cell = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3;
    for (int i = 0; i < kMotifCount; ++i) {
        if (i % 3 != 0) {
            ImGui::SameLine();
        }
        if (ImGui::Selectable(kMotifNames[i], settings.motifIndex == i, 0,
                              ImVec2(cell, cell * 0.6f))) {
            settings.motifIndex = i;
        }
    }

    sectionLabel("Symmetry rule");
    for (int i = 0; i < 3; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        if (ImGui::RadioButton(toString(kRules[i]), settings.rule == kRules[i])) {
            settings.rule = kRules[i];
        }
    }

    sectionLabel("Tile size");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##tile", &settings.tileSize, 40, 300, "%d px");

    sectionLabel("Spacing");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##spacing", &settings.spacing, 0, 60, "%d px");

    // Placeholder swatches using ImGui's picker; our own RGB / HSV picker
    // replaces this in the interface and colour work.
    sectionLabel("Palette");
    const ImGuiColorEditFlags swatch =
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha;
    for (std::size_t i = 0; i < settings.palette.size(); ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        ImVec4 value = toImVec4(settings.palette[i]);
        const std::string id = "##palette" + std::to_string(i);
        if (ImGui::ColorEdit4(id.c_str(), &value.x, swatch)) {
            settings.palette[i] = fromImVec4(value);
        }
    }

    sectionLabel("Background");
    ImVec4 background = toImVec4(settings.background);
    if (ImGui::ColorEdit4("##background", &background.x, swatch)) {
        settings.background = fromImVec4(background);
    }
}

}  // namespace

StudioFrame drawStudio(PatternSettings& settings, const CanvasView& canvas,
                       const StudioStatus& status) {
    StudioFrame frame;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Kitenge", nullptr, flags);

    frame.actions = drawTopBar();

    const float scale = ImGui::GetStyle().FontScaleDpi;
    ImGui::BeginChild("controls", ImVec2(kControlsWidth * scale, 0), ImGuiChildFlags_Borders);
    drawControls(settings);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::TextDisabled("Pattern canvas");
    const float statusHeight = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    frame.canvasWidth = std::max(0, static_cast<int>(avail.x));
    frame.canvasHeight = std::max(0, static_cast<int>(avail.y - statusHeight));

    if (canvas.texture()) {
        ImGui::Image(
            static_cast<ImTextureID>(reinterpret_cast<std::intptr_t>(canvas.texture())),
            ImVec2(static_cast<float>(canvas.width()), static_cast<float>(canvas.height())));
    } else {
        ImGui::Dummy(
            ImVec2(static_cast<float>(frame.canvasWidth), static_cast<float>(frame.canvasHeight)));
    }

    ImGui::TextDisabled("%s - tile %d px - %d tiles drawn - all primitives rasterised in-house",
                        toString(settings.rule), settings.tileSize, status.tilesDrawn);
    ImGui::EndGroup();

    ImGui::End();
    return frame;
}

}  // namespace kitenge
