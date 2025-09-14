#include "application.hpp"

#include "SDL3/SDL_timer.h"
#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "platform/platform.hpp"

struct App_State {
	b8 is_running;
};

global_variable App_State state = {};

b8 application_init() {

    if (!log_init()) {
        CORE_FATAL("Failed to initialize log subsystem");
        return false;
    }

    if (!platform_startup(
            "Prometheus client",
            1280,
            720)) {
        CORE_FATAL("Failed to initialize platform subsystem");
        return false;
    }

    CORE_INFO("Subsystems initialized correctly.");

    CORE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return true;
}

void application_run() {
	state.is_running = true;

	while(state.is_running) {
		if(!platform_message_pump()) {
			state.is_running = false;
		}
	}

    application_shutdown();
}

void application_shutdown() {
    platform_shutdown();
    log_shutdown();
}
