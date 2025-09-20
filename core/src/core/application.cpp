#include "application.hpp"

#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer_backend.hpp"
#include "ui/ui.hpp"
#include "assets/assets.hpp"

struct App_State {
    b8 is_running;
    b8 is_suspended;
    Platform_State plat_state;
    App_Config config;
};

global_variable App_State state = {};

b8 application_init(const App_Config* config) {
    RUNTIME_ASSERT_MSG(config, "Application config cannot be null");

    // Store configuration
    state.config = *config;

    if (!log_init()) {
        CORE_FATAL("Failed to initialize log subsystem");
        return false;
    }

    if (!platform_startup(
            &state.plat_state,
            state.config.name,
            state.config.width,
            state.config.height)) {
        CORE_FATAL("Failed to initialize platform subsystem");
        return false;
    }

    if (!renderer_initialize()) {
        CORE_FATAL("Failed to initialize renderer");
        return false;
    }

    if (!assets_initialize()) {
        CORE_FATAL("Failed to initialize asset system");
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
    assets_shutdown();
    renderer_shutdown();
    platform_shutdown();
    log_shutdown();

    CORE_INFO("All subsystems shut down correctly.");
}

const App_Config* application_get_config() {
    return &state.config;
}

App_Config application_get_default_config() {
    App_Config config = {};
    config.name = "Prometheus Engine";
    config.width = 1280;
    config.height = 720;
    return config;
}

b8 application_set_client_ui_config(Client_UI_Config* client_config) {
    if (!client_config) {
        CORE_ERROR("Client UI config cannot be null");
        return false;
    }

    // Pass the client configuration to the UI system
    return ui_set_client_config(client_config);
}

