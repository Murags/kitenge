# Third-party dependencies.
#
# SDL2 is only used to open a window, read input and show our framebuffer;
# all drawing is done by our own rasteriser. A system SDL2 is used when one
# is installed (e.g. apt install libsdl2-dev), otherwise it is downloaded and
# built from source so the project builds the same way on every machine.

include(FetchContent)

find_package(SDL2 CONFIG QUIET)

if(SDL2_FOUND)
  message(STATUS "Using system SDL2 ${SDL2_VERSION}")
  set(KITENGE_SDL_TARGET SDL2::SDL2)
else()
  message(STATUS "System SDL2 not found, fetching it")
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(SDL2)
  set(KITENGE_SDL_TARGET SDL2::SDL2-static)
endif()

# Dear ImGui draws the control panel only (buttons, sliders, colour picker).
# It has no CMake build of its own, so we compile the core files and the
# SDL2 + SDL_Renderer backends into a small static library.
FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.9.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC ${KITENGE_SDL_TARGET})
