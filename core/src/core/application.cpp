#include "application.hpp"

#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/vulkan_backend.hpp"

struct App_State {
	b8 is_running;
	b8 is_suspended;
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

	if(!renderer_initialize()){
        CORE_FATAL("Failed to initialize renderer");
        return false;
	}

	state.is_running = false;
	state.is_suspended = false;

    CORE_INFO("Subsystems initialized correctly.");

    CORE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return true;
}

void application_run() {
	state.is_running = true;
	b8 show_demo = true;

	while(state.is_running) {
		// For each iteration read the new messages from the queue
		if(!platform_message_pump()) {
			state.is_running = false;
		}

		// Frame
		if(!state.is_suspended) {
			if(!renderer_draw_frame(&show_demo)) {
				state.is_running = false;
			}
		}
	}

    application_shutdown();
}

void application_shutdown() {
	renderer_shutdown();
    platform_shutdown();
    log_shutdown();
}
