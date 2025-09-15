#include "platform.hpp"

#include <stdlib.h>
#include <string.h>

#include "SDL3/SDL_vulkan.h"
#include "imgui_impl_sdl3.h"

#include "containers/auto_array.hpp"
#include "core/logger.hpp"
#include "renderer/vulkan_types.hpp"

internal_variable Platform_State* state_ptr = nullptr;

b8 platform_startup(
    Platform_State* state,
    const char* application_name,
    s32 width,
    s32 height) {

	state_ptr = state;

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
    state_ptr->main_scale = main_scale;

    SDL_WindowFlags window_flags =
		SDL_WINDOW_VULKAN |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIGH_PIXEL_DENSITY;

    state_ptr->window = SDL_CreateWindow(
        application_name,
        width,
        height,
        window_flags);

    if (state_ptr->window == nullptr) {
        CORE_ERROR(
            "SDL_CreateWindow() failed with message: '%s'",
            SDL_GetError());

        return false;
    }

    CORE_DEBUG("Window created successfully");

    SDL_SetWindowPosition(state_ptr->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(state_ptr->window);

    CORE_DEBUG("Window positioned and shown");
    CORE_INFO("Platform subsystem initialized successfully");

    return true;
}

void platform_shutdown() {

    if (state_ptr != nullptr && state_ptr->window) {
        SDL_DestroyWindow(state_ptr->window);
        state_ptr->window = nullptr;
    }

    SDL_Quit();
}

b8 platform_message_pump() {
    SDL_Event event;

    b8 quit_flagged = false;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        
        switch (event.type) {
		case SDL_EVENT_QUIT:
            quit_flagged = true;
            break; // Signal application should quit

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            quit_flagged = true;
            break; // Window close button clicked

		case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE) {
                quit_flagged = true;
            }
            break;

		case SDL_EVENT_WINDOW_RESIZED:
            // Handle window resize if needed
            break;

		default: 
            // Handle other events as needed
            break;
        }
    }

    return !quit_flagged;
}

void platform_get_vulkan_extensions(Auto_Array<const char*>* extensions) {
    uint32_t sdl_extensions_count = 0;

    const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(
        &sdl_extensions_count);

    for (uint32_t n = 0; n < sdl_extensions_count; n++)
        extensions->push_back(sdl_extensions[n]);
}

b8 platform_create_vulkan_surface(
	Vulkan_Context* context){

	if(!state_ptr) return false;

    if (SDL_Vulkan_CreateSurface(
			state_ptr->window, 
			context->instance, 
			context->allocator, 
			&context->surface) == 0)
    {
        CORE_ERROR("Failed to create Vulkan surface.");
        return false;
    }

	return true;
}

b8 platform_get_window_details(
	u32* width,
	u32* height,
	f32* main_scale) {

	int w, h;
	
    SDL_GetWindowSize(state_ptr->window, &w, &h);

	*width = w;
	*height = h;
	*main_scale = state_ptr->main_scale;

	return true;
}

void platform_init_vulkan() {
    ImGui_ImplSDL3_InitForVulkan(state_ptr->window);
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
