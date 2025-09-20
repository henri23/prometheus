#pragma once

#include "defines.hpp"

#include <SDL3/SDL.h>

struct Platform_State {
    SDL_Window* window;
    SDL_Renderer* renderer;
	f32 main_scale;
};

// Event callback type for UI subsystem integration
typedef b8 (*Platform_EventCallback)(const SDL_Event* event);

b8 platform_startup(
	Platform_State* state,
    const char* application_name,
    s32 width,
    s32 height);

void platform_shutdown();

b8 platform_message_pump();

void* platform_allocate(u64 size, b8 aligned);

void platform_free(void* block, b8 aligned);

void* platform_zero_memory(void* block, u64 size);

void* platform_copy_memory(void* dest, const void* source, u64 size);

void* platform_move_memory(void* dest, const void* source, u64 size);

void* platform_set_memory(void* dest, s32 value, u64 size);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);

// Event callback registration for UI subsystem
void platform_set_event_callback(Platform_EventCallback callback);

// Temporary accessor for UI subsystem (TODO: improve architecture)
Platform_State* get_platform_state();

// Window control functions
b8 platform_get_window_details(u32* width, u32* height, f32* main_scale);
void platform_minimize_window();
void platform_maximize_window();
void platform_restore_window();
void platform_close_window();
b8 platform_is_window_maximized();
