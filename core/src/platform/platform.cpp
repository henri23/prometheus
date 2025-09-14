#include "platform.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.hpp"

#include <SDL3/SDL.h>

struct Platform_State {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

internal_variable Platform_State state = {};

b8 platform_startup(
    const char* application_name,
    s32 width,
    s32 height) {

    CORE_DEBUG("Starting platform subsystem...");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        CORE_ERROR(
            "SDL_Init() failed with message:'%s'",
            SDL_GetError());

        return false;
    }

    CORE_DEBUG("SDL initialized successfully");

    // Create window with Vulkan graphics context
    f32 main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    SDL_WindowFlags window_flags =
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIGH_PIXEL_DENSITY;

    state.window = SDL_CreateWindow(
        application_name,
        width,
        height,
        window_flags);

    if (state.window == nullptr) {
        CORE_ERROR(
            "SDL_CreateWindow() failed with message: '%s'",
            SDL_GetError());

        return false;
    }

    CORE_DEBUG("Window created successfully");

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer) {
        CORE_FATAL("Failed to create renderer with message: '%s'", SDL_GetError());

        SDL_DestroyWindow(state.window);
        SDL_Quit();

        return false;
    }
    
    SDL_SetWindowPosition(state.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(state.window);

    CORE_DEBUG("Window positioned and shown");
    CORE_INFO("Platform subsystem initialized successfully");

    return true;
}

void platform_shutdown() {
    if (state.renderer) {
        SDL_DestroyRenderer(state.renderer);
        state.renderer = nullptr;
    }
    
    if (state.window) {
        SDL_DestroyWindow(state.window);
        state.window = nullptr;
    }
    
    SDL_Quit();
}

b8 platform_message_pump() {
    SDL_Event event;

    b8 quit_flagged = false;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            quit_flagged = true;
            break; // Signal application should quit

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            quit_flagged = true;
            break; // Window close button clicked

        case SDL_EVENT_KEY_DOWN:
            // Handle key presses if needed
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            // Handle window resize if needed
            break;

        default:
            // Handle other events as needed
            break;
        }
    }
    
    // Basic rendering - essential for window visibility on Wayland/Hyprland
    if (state.renderer) {
        // Clear with dark gray color
        SDL_SetRenderDrawColor(state.renderer, 64, 64, 64, 255);
        SDL_RenderClear(state.renderer);
        
        // Present the rendered frame
        SDL_RenderPresent(state.renderer);
    }

    return !quit_flagged;
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void* block, b8 aligned) {
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_move_memory(void* dest, const void* source, u64 size) {
    return memmove(dest, source, size);
}

void* platform_set_memory(void* dest, s32 value, u64 size) {
    return memset(dest, value, size);
}

// f64 platform_get_absolute_time() {
// }
