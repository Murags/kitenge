#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <cstdio>

#include "core/Framebuffer.h"
#include "core/PatternSettings.h"
#include "ui/CanvasView.h"
#include "ui/StudioLayout.h"

namespace {

constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 800;

// Redraws the whole pattern into the framebuffer. Until the symmetry engine
// is in place the canvas is only cleared to the background colour.
void renderPattern(kitenge::Framebuffer& fb, const kitenge::PatternSettings& settings,
                   kitenge::StudioStatus& status) {
    fb.clear(settings.background);
    status.tilesDrawn = 0;
}

}  // namespace

int main(int, char**) {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const float uiScale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_Window* window = SDL_CreateWindow(
        "Kitenge - pattern studio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        static_cast<int>(kWindowWidth * uiScale), static_cast<int>(kWindowHeight * uiScale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;  // the layout is fixed, so don't write imgui.ini

    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(uiScale);
    style.FontScaleDpi = uiScale;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    {
        kitenge::PatternSettings settings;
        kitenge::StudioStatus status;
        kitenge::Framebuffer framebuffer;
        kitenge::CanvasView canvas(renderer);
        int canvasWidth = 0;
        int canvasHeight = 0;
        bool dirty = true;

        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window)) {
                    running = false;
                }
            }
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
                SDL_Delay(10);
                continue;
            }

            // Resize to the size the layout asked for last frame, then redraw the
            // pattern only when something has changed.
            if (canvasWidth != framebuffer.width() || canvasHeight != framebuffer.height()) {
                framebuffer.resize(canvasWidth, canvasHeight);
                dirty = true;
            }
            if (dirty) {
                renderPattern(framebuffer, settings, status);
                canvas.upload(framebuffer);
                dirty = false;
            }

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            const kitenge::PatternSettings before = settings;
            const kitenge::StudioFrame frame = kitenge::drawStudio(settings, canvas, status);
            canvasWidth = frame.canvasWidth;
            canvasHeight = frame.canvasHeight;
            if (settings != before) {
                dirty = true;
            }

            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x,
                               io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, 0xFA, 0xEC, 0xDA, 0xFF);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);
        }
    }  // the canvas texture must be released before the renderer

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
