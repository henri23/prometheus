#include "application.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "core/logger.hpp"
#include "memory/memory.hpp"

struct App_State {
};

b8 application_init() {

    if (!log_init()) {
        return false;
    }

    CORE_DEBUG("Subsystems initialized correctly.");

    CORE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated
	
    CORE_INFO("Test");
    CORE_DEBUG("Test");
    CORE_WARN("Test");
    CORE_ERROR("Test");
    CORE_FATAL("Test");
    CORE_TRACE("Test %d", 5);
    return true;
}

b8 application_run() {
    application_shutdown();

    return true;
}

void application_shutdown() {
    log_shutdown();
}
