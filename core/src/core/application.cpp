#include "application.hpp"

#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer_backend.hpp"
#include "ui/ui.hpp"

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

    if (!renderer_initialize()) {
        CORE_FATAL("Failed to initialize renderer");
        return false;
    }

    if (!ui_initialize()) {
        CORE_FATAL("Failed to initialize UI subsystem");
        return false;
    }

    // Register UI event callback with platform
    platform_set_event_callback(ui_process_event);

    state.is_running = false;
    state.is_suspended = false;

    CORE_INFO("All subsystems initialized correctly.");

    CORE_DEBUG(memory_get_current_usage()); // WARN: Memory leak because the heap allocated string must be deallocated

    return true;
}

void application_run() {
    state.is_running = true;

    while (state.is_running) {
        // Process platform events (will forward to UI via callback)
        if (!platform_message_pump()) {
            state.is_running = false;
        }

        // Render frame if not suspended
        if (!state.is_suspended) {
            // Start new UI frame
            ui_new_frame();

            // Render the frame
            if (!renderer_draw_frame(ui_render())) {
                state.is_running = false;
            }
        }
    }

    application_shutdown();
}

void application_shutdown() {
    ui_shutdown();
    renderer_shutdown();
    platform_shutdown();
    log_shutdown();

    CORE_INFO("All subsystems shut down correctly.");
}
