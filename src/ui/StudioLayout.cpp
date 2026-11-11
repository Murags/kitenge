#include "ui/StudioLayout.h"

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "motifs/MotifLibrary.h"

namespace kitenge {

namespace {

constexpr SymmetryRule kRules[] = {SymmetryRule::Translate, SymmetryRule::FourFold,
                                   SymmetryRule::Mirror};

constexpr float kControlsWidth = 300.0f;

ImVec4 toImVec4(Color c) {
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
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

    actions.animate = ImGui::Button(labels[0]);
    ImGui::SetItemTooltip("Replay the pattern assembling itself, one copy at a time");
    ImGui::SameLine();
    // The 3D preview is a stretch goal.
    ImGui::BeginDisabled();
    actions.preview3d = ImGui::Button(labels[1]);
    ImGui::EndDisabled();
    ImGui::SameLine();
    actions.exportImage = ImGui::Button(labels[2]);
    ImGui::SetItemTooltip("Save the canvas as a BMP image");

    ImGui::Separator();
    return actions;
}

// A clickable colour swatch, outlined when it is the one being edited.
bool swatch(const char* id, Color color, bool selected) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float size = ImGui::GetFrameHeight();
    const bool clicked =
        ImGui::ColorButton(id, toImVec4(color), ImGuiColorEditFlags_NoTooltip, ImVec2(size, size));
    if (selected) {
        ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x - 2, pos.y - 2),
                                            ImVec2(pos.x + size + 2, pos.y + size + 2),
                                            IM_COL32(0, 0, 0, 255), 0.0f, 0, 2.0f);
    }
    return clicked;
}

void drawColors(PatternSettings& settings, ColorPicker& picker) {
    const int count = static_cast<int>(settings.palette.size());
    picker.target = std::clamp(picker.target, 0, count);

    sectionLabel("Palette");
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        ImGui::PushID(i);
        if (swatch("##palette", settings.palette[static_cast<std::size_t>(i)],
                   picker.target == i)) {
            picker.target = i;
        }
        ImGui::PopID();
    }

    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 4);
    if (swatch("##background", settings.background, picker.target == count)) {
        picker.target = count;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("background");

    ImGui::Spacing();
    Color& editing = picker.target == count
                         ? settings.background
                         : settings.palette[static_cast<std::size_t>(picker.target)];
    picker.draw(editing);
}

void drawControls(PatternSettings& settings, ColorPicker& picker) {
    sectionLabel("Motif");
    const auto& motifs = builtinMotifs();
    const float cell = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3;
    for (int i = 0; i < static_cast<int>(motifs.size()); ++i) {
        if (i % 3 != 0) {
            ImGui::SameLine();
        }
        if (ImGui::Selectable(motifs[static_cast<std::size_t>(i)].name.c_str(),
                              settings.motifIndex == i, 0, ImVec2(cell, cell * 0.6f))) {
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

    drawColors(settings, picker);

    sectionLabel("Rasterisation");
    ImGui::Checkbox("Fill", &settings.render.fill);
    ImGui::SameLine();
    ImGui::Checkbox("Outline", &settings.render.outline);
}

}  // namespace

StudioFrame drawStudio(PatternSettings& settings, const CanvasView& canvas,
                       const StudioStatus& status, ColorPicker& picker) {
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
    drawControls(settings, picker);
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

    ImGui::TextDisabled("%s - tile %d px - %d tiles drawn - all primitives rasterised in-house%s%s",
                        toString(settings.rule), settings.tileSize, status.tilesDrawn,
                        status.message.empty() ? "" : " - ", status.message.c_str());
    ImGui::EndGroup();

    ImGui::End();
    return frame;
}

}  // namespace kitenge
