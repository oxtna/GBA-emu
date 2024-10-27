#include <SDL2/SDL.h>
#include <iostream>

int main() {
    std::cout << "Hello, world!" << std::endl;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL2 library\n";
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if (window == NULL) {
        std::cerr << "Failed to create window\n";
        return -1;
    }
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (surface == NULL) {
        std::cerr << "Failed to get the surface of the window\n";
        return -1;
    }
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e) > 0) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }
        }
        SDL_UpdateWindowSurface(window);
    }
}

