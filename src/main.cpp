#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <cstdio>

int main(int, char**) {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Quit();
    return 0;
}
