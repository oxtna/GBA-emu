#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>

void handleKeyEvent(const SDL_Event& e) {
    switch (e.key.keysym.sym) {
    case SDLK_UP: {
        std::cerr << "UP key pressed\n";
        break;
    }
    case SDLK_DOWN: {
        std::cerr << "DOWN key pressed\n";
        break;
    }
    case SDLK_LEFT: {
        std::cerr << "LEFT key pressed\n";
        break;
    }
    case SDLK_RIGHT: {
        std::cerr << "RIGHT key pressed\n";
        break;
    }
    case SDLK_q: {
        std::cerr << "Q key pressed\n";
        break;
    }
    case SDLK_w: {
        std::cerr << "W key pressed\n";
        break;
    }
    case SDLK_a: {
        std::cerr << "A key pressed\n";
        break;
    }
    case SDLK_s: {
        std::cerr << "S key pressed\n";
        break;
    }
    case SDLK_z: {
        std::cerr << "Z key pressed\n";
        break;
    }
    case SDLK_x: {
        std::cerr << "X key pressed\n";
        break;
    }
    }
}

int main() {
    std::cout << "Hello, world!" << std::endl;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL2 library\n";
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if (window == NULL) {
        std::cerr << "Failed to create window: " << SDL_GetError() << '\n';
        SDL_Quit();
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
    if (texture == NULL) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    int* display_array = (int*)malloc(sizeof(int) * 240 * 160);
    if (display_array == NULL) {
        std::cerr << "Failed to allocate memory for display array\n";
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool running = true;
    while (running) {
        auto current_time = SDL_GetTicks();
        // TODO: emulate n instructions
        // TODO: render here
        SDL_Event e;
        while (SDL_PollEvent(&e) > 0) {
            switch (e.type) {
            case SDL_KEYDOWN: {
                handleKeyEvent(e);
                break;
            }
            case SDL_QUIT: {
                running = false;
                break;
            }
            default:
                break;
            }
        }
        SDL_RenderClear(renderer);
        //
        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);
        auto elapsed_time = SDL_GetTicks() - current_time;
        if (elapsed_time < 16) {
            SDL_Delay(16 - elapsed_time);
        }
    }
    free(display_array);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindowSurface(window);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
