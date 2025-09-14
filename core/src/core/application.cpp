#include "application.hpp"

#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/vulkan_backend.hpp"

struct App_State {
	b8 is_running;
	Platform_State plat_state;
};

global_variable App_State state = {};

b8 application_init() {

    if (!log_init()) {
        CORE_FATAL("Failed to initialize log subsystem");
        return false;
    }

    if (!platform_startup(
			&state.plat_state,
            "Prometheus client",
            1280,
            720)) {
        CORE_FATAL("Failed to initialize platform subsystem");
        return false;
    }

	if(!renderer_initialize(&state.plat_state)){
        CORE_FATAL("Failed to initialize renderer");
        return false;
	}

    CORE_INFO("Subsystems initialized correctly.");

    CORE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return true;
}

void application_run() {
	state.is_running = true;

	while(state.is_running) {
		if(!platform_message_pump(&state.plat_state)) {
			state.is_running = false;
		}
	}

    application_shutdown();
}

void application_shutdown() {
    platform_shutdown(&state.plat_state);
    log_shutdown();
}
