#include "common.h"
#include "emulator.h"
#include <SDL2/SDL.h>
#include <vector>

void handleKeyEvent(const SDL_Event& event) {
    switch (event.key.keysym.sym) {
    case SDLK_UP: {
        std::fprintf(stderr, "UP key pressed\n");
        break;
    }
    case SDLK_DOWN: {
        std::fprintf(stderr, "DOWN key pressed\n");
        break;
    }
    case SDLK_LEFT: {
        std::fprintf(stderr, "LEFT key pressed\n");
        break;
    }
    case SDLK_RIGHT: {
        std::fprintf(stderr, "RIGHT key pressed\n");
        break;
    }
    case SDLK_q: {
        std::fprintf(stderr, "Q key pressed\n");
        break;
    }
    case SDLK_w: {
        std::fprintf(stderr, "W key pressed\n");
        break;
    }
    case SDLK_a: {
        std::fprintf(stderr, "A key pressed\n");
        break;
    }
    case SDLK_s: {
        std::fprintf(stderr, "S key pressed\n");
        break;
    }
    case SDLK_z: {
        std::fprintf(stderr, "Z key pressed\n");
        break;
    }
    case SDLK_x: {
        std::fprintf(stderr, "X key pressed\n");
        break;
    }
    default:
        break;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: GBA_Emu ROM\n");
        return -1;
    }
    GBA::Emulator emulator;
    {
        FILE* rom_file = std::fopen(argv[1], "rb");
        if (rom_file == NULL) {
            std::perror("Failed to open file");
            return -1;
        }
        std::fseek(rom_file, 0, SEEK_END);
        auto rom_size = std::ftell(rom_file);
        std::rewind(rom_file);
        if (rom_size < 0) {
            std::fprintf(stderr, "Failed to determine ROM size\n");
            return -1;
        }
        std::vector<uint8_t> rom_buffer(rom_size);
        if (std::fread(rom_buffer.data(), sizeof(uint8_t), rom_buffer.size(), rom_file) != rom_size) {
            std::fprintf(stderr, "Failed to read the entire file\n");
            return -1;
        }
        std::fclose(rom_file);
        emulator.loadBIOS(std::move(rom_buffer));
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "Failed to initialize SDL2 library: %s\n", SDL_GetError());
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if (window == NULL) {
        std::fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
    if (texture == NULL) {
        std::fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    uint32_t* framebuffer = static_cast<uint32_t*>(std::malloc(sizeof(uint32_t) * 240 * 160));
    if (framebuffer == NULL) {
        std::perror("Failed to allocate memory for framebuffer");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool running = true;
    while (running) {
        auto current_time = SDL_GetTicks();
        SDL_Event event;
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                handleKeyEvent(event);
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
        // TODO: emulate n instructions (advance N clock cycles?)
        emulator.step();  // how many is N?
        SDL_RenderClear(renderer);
        auto [displayBufferStart, displayBufferEnd] = emulator.getDisplay();
        // TODO: render
        for(auto it = displayBufferStart; it != displayBufferEnd; it += 4) {

            uint8_t r = *(it);
            uint8_t g = *(it + 1);
            uint8_t b = *(it + 2);
            uint8_t a = *(it + 3);

            framebuffer[(it - displayBufferStart) / 4] = (a << 24) | (r << 16) | (g << 8) | b;
        }

        SDL_UpdateTexture(texture, NULL, framebuffer, 240 * sizeof(uint32_t));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);
        auto elapsed_time = SDL_GetTicks() - current_time;
        // 16 milliseconds per frame for approximately 60 fps
        if (elapsed_time < 16) {
            SDL_Delay(16 - elapsed_time);
        }
    }

    std::free(framebuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
