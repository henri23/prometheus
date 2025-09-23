#include "platform.hpp"

#include <stdlib.h>
#include <string.h>

#include "SDL3/SDL_vulkan.h"

#include "containers/auto_array.hpp"
#include "core/logger.hpp"
#include "events/events.hpp"
#include "input/input.hpp"
#include "input/input_codes.hpp"
#include "renderer/vulkan/vulkan_types.hpp"

internal_variable Platform_State* state_ptr = nullptr;

// Internal function to translate SDL events to engine events
INTERNAL_FUNC void translate_sdl_event(const SDL_Event* sdl_event) {
    Event engine_event = {};

    switch (sdl_event->type) {
    case SDL_EVENT_KEY_DOWN:
        engine_event.type = Event_Type::KEY_PRESSED;
        engine_event.key.key_code = platform_to_key_code(sdl_event->key.scancode);
        engine_event.key.repeat = sdl_event->key.repeat;
        events_dispatch(&engine_event);
        input_process_key(platform_to_key_code(sdl_event->key.scancode), true);
        break;

    case SDL_EVENT_KEY_UP:
        engine_event.type = Event_Type::KEY_RELEASED;
        engine_event.key.key_code = platform_to_key_code(sdl_event->key.scancode);
        engine_event.key.repeat = false;
        events_dispatch(&engine_event);
        input_process_key(platform_to_key_code(sdl_event->key.scancode), false);
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        engine_event.type = Event_Type::MOUSE_BUTTON_PRESSED;
        engine_event.mouse_button.button = platform_to_mouse_button(sdl_event->button.button);
        engine_event.mouse_button.x = (s32)sdl_event->button.x;
        engine_event.mouse_button.y = (s32)sdl_event->button.y;
        events_dispatch(&engine_event);
        input_process_mouse_button(platform_to_mouse_button(sdl_event->button.button), true);
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        engine_event.type = Event_Type::MOUSE_BUTTON_RELEASED;
        engine_event.mouse_button.button = platform_to_mouse_button(sdl_event->button.button);
        engine_event.mouse_button.x = (s32)sdl_event->button.x;
        engine_event.mouse_button.y = (s32)sdl_event->button.y;
        events_dispatch(&engine_event);
        input_process_mouse_button(platform_to_mouse_button(sdl_event->button.button), false);
        break;

    case SDL_EVENT_MOUSE_MOTION:
        engine_event.type = Event_Type::MOUSE_MOVED;
        engine_event.mouse_move.x = (s32)sdl_event->motion.x;
        engine_event.mouse_move.y = (s32)sdl_event->motion.y;
        engine_event.mouse_move.delta_x = (s32)sdl_event->motion.xrel;
        engine_event.mouse_move.delta_y = (s32)sdl_event->motion.yrel;
        events_dispatch(&engine_event);
        input_process_mouse_move((s32)sdl_event->motion.x, (s32)sdl_event->motion.y);
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        engine_event.type = Event_Type::MOUSE_WHEEL_SCROLLED;
        engine_event.mouse_wheel.x = (s32)sdl_event->wheel.mouse_x;
        engine_event.mouse_wheel.y = (s32)sdl_event->wheel.mouse_y;
        engine_event.mouse_wheel.delta_x = sdl_event->wheel.x;
        engine_event.mouse_wheel.delta_y = sdl_event->wheel.y;
        events_dispatch(&engine_event);
        input_process_mouse_wheel(sdl_event->wheel.x, sdl_event->wheel.y);
        break;

    case SDL_EVENT_WINDOW_RESIZED:
        engine_event.type = Event_Type::WINDOW_RESIZED;
        engine_event.window_resize.width = (u32)sdl_event->window.data1;
        engine_event.window_resize.height = (u32)sdl_event->window.data2;
        events_dispatch(&engine_event);
        break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        engine_event.type = Event_Type::WINDOW_CLOSED;
        events_dispatch(&engine_event);
        break;

    case SDL_EVENT_WINDOW_MINIMIZED:
        engine_event.type = Event_Type::WINDOW_MINIMIZED;
        events_dispatch(&engine_event);
        break;

    case SDL_EVENT_WINDOW_MAXIMIZED:
        engine_event.type = Event_Type::WINDOW_MAXIMIZED;
        events_dispatch(&engine_event);
        break;

    case SDL_EVENT_WINDOW_RESTORED:
        engine_event.type = Event_Type::WINDOW_RESTORED;
        events_dispatch(&engine_event);
        break;
    }
}

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

    SDL_SetWindowPosition(
        state_ptr->window,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(state_ptr->window);

    CORE_DEBUG("Window positioned and shown");
    CORE_INFO("Platform subsystem initialized successfully");

    return true;
}

void platform_shutdown() {
	CORE_DEBUG("Platform shutting down...");

    if (state_ptr != nullptr && state_ptr->window) {
        SDL_DestroyWindow(state_ptr->window);
        state_ptr->window = nullptr;
    }

    SDL_Quit();
	CORE_DEBUG("Platform shut down.");
}

b8 platform_message_pump() {
    SDL_Event event;

    b8 quit_flagged = false;

    while (SDL_PollEvent(&event)) {
        // Translate SDL event to engine event and dispatch to input system
        translate_sdl_event(&event);

        // Process platform-level events
        {
            switch (event.type) {
		case SDL_EVENT_QUIT:
            quit_flagged = true;
            break; // Signal application should quit

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            quit_flagged = true;
            break; // Window close button clicked

		case SDL_EVENT_KEY_DOWN:
            // ESC key handling moved to application layer
            break;

		case SDL_EVENT_WINDOW_RESIZED:
            // TODO: Trigger swapchain recreation when window is resized
            CORE_DEBUG("Window resized - triggered swapchain recreation");
            break;

		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_RESTORED:
            // TODO: Trigger swapchain recreation when window is maximized or
            // restored
            CORE_DEBUG(
                "Window state changed - triggered swapchain recreation");
            break;

		default:
            // Handle other events as needed
            break;
            }
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

f64 platform_get_absolute_time() {
    return (f64)SDL_GetTicksNS() / 1000000000.0; // Convert nanoseconds to seconds
}

void platform_sleep(u64 ms) {
    SDL_Delay((u32)ms);
}



Platform_State* get_platform_state() {
    return state_ptr;
}

void platform_minimize_window() {
    if (state_ptr && state_ptr->window) {
        SDL_MinimizeWindow(state_ptr->window);
        CORE_DEBUG("Window minimized");
    }
}

void platform_maximize_window() {
    if (state_ptr && state_ptr->window) {
        SDL_MaximizeWindow(state_ptr->window);
        CORE_DEBUG("Window maximized");
    }
}

void platform_restore_window() {
    if (state_ptr && state_ptr->window) {
        SDL_RestoreWindow(state_ptr->window);
        CORE_DEBUG("Window restored");
    }
}

void platform_close_window() {
    if (state_ptr && state_ptr->window) {
        SDL_Event quit_event;
        quit_event.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit_event);
        CORE_DEBUG("Window close requested");
    }
}

b8 platform_is_window_maximized() {
    if (state_ptr && state_ptr->window) {
        SDL_WindowFlags flags = SDL_GetWindowFlags(state_ptr->window);
        return (flags & SDL_WINDOW_MAXIMIZED) != 0;
    }
    return false;
}

void platform_get_required_extensions(Auto_Array<const char*>* required_extensions) {
    // Get the extensions needed by SDL3 for Vulkan
    Uint32 extension_count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);

    if (!extensions) {
        CORE_ERROR("Failed to get Vulkan instance extensions from SDL3: %s", SDL_GetError());
        return;
    }

    // Add all required extensions to the array
    for (Uint32 i = 0; i < extension_count; ++i) {
        required_extensions->push_back(extensions[i]);
        CORE_DEBUG("Required Vulkan extension: %s", extensions[i]);
    }

    CORE_DEBUG("Added %u Vulkan extensions from SDL3", extension_count);
}
