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
