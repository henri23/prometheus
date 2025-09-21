#include "application.hpp"

#include "assets/assets.hpp"
#include "client_types.hpp"
#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer_backend.hpp"
#include "ui/ui.hpp"

// Application configuration
constexpr u32 TARGET_FPS = 120;
constexpr f64 TARGET_FRAME_TIME = 1 / (f64)TARGET_FPS;

struct App_State {
    Client_State* client_state;
    b8 is_running;
    b8 is_suspended;
    Platform_State plat_state;
};

// Internal pointer to application state for easy access
internal_variable App_State* application_state = nullptr;

b8 application_init(Client_State* client_state) {
    RUNTIME_ASSERT_MSG(client_state, "Client state cannot be null");

    // Protect against multiple initialization
    if (client_state->application_state != nullptr) {
        CORE_ERROR("Application already initialized");
        return false;
    }

    // Allocate application state
    client_state->application_state =
        memory_allocate(sizeof(App_State), Memory_Tag::APPLICATION);

    application_state =
        static_cast<App_State*>(client_state->application_state);

    application_state->client_state = client_state;

    if (!log_init()) {
        CORE_FATAL("Failed to initialize log subsystem");
        return false;
    }

    if (!platform_startup(&application_state->plat_state,
            client_state->config.name,
            client_state->config.width,
            client_state->config.height)) {
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

    // Initialize UI with configuration from client
    if (!ui_initialize(
            UI_Theme::CATPPUCCIN_MOCHA,
            client_state->config.use_dockspace,
            client_state->config.custom_titlebar)) {
        CORE_FATAL("Failed to initialize UI subsystem");
        return false;
    }

    // Register UI event callback with platform
    platform_set_event_callback(ui_process_event);

    application_state->is_running = false;
    application_state->is_suspended = false;

    CORE_INFO("All subsystems initialized correctly.");

    CORE_DEBUG(
        memory_get_current_usage()); // WARN: Memory leak because the heap
                                     // allocated string must be deallocated

    return true;
}

void application_run() {
    if (!application_state) {
        CORE_FATAL("Application not initialized");
        return;
    }

    application_state->is_running = true;

    // Call client initialize if provided
    if (application_state->client_state->initialize) {
        if (!application_state->client_state->initialize(
                application_state->client_state)) {
            CORE_ERROR("Client initialization failed");
            return;
        }
    }

    // Frame rate limiting variables
    f64 frame_start_time = platform_get_absolute_time();

    while (application_state->is_running) {
        f64 current_time = platform_get_absolute_time();
        f64 delta_time = current_time - frame_start_time;
        frame_start_time = current_time;

        // Process platform events (will forward to UI via callback)
        if (!platform_message_pump()) {
            application_state->is_running = false;
        }

        // Call client update if provided
        if (application_state->client_state->update) {
            if (!application_state->client_state->update(
                    application_state->client_state,
                    delta_time)) {
                application_state->is_running = false;
            }
        }

        // Render frame if not suspended
        if (!application_state->is_suspended) {
            // Start new UI frame
            ui_new_frame();

            // Call client render if provided
            if (application_state->client_state->render) {
                application_state->client_state->render(
                    application_state->client_state,
                    delta_time);
            }

            // Render the frame
            if (!renderer_draw_frame(ui_render())) {
                application_state->is_running = false;
            }
        }

        // Frame rate limiting

        f64 frame_end_time = platform_get_absolute_time();
        f64 frame_duration = frame_end_time - current_time;

        if (frame_duration < TARGET_FRAME_TIME) {
            u64 sleep_ms = (u64)((TARGET_FRAME_TIME - frame_duration) * 1000.0);

            if (sleep_ms > 0) {
                platform_sleep(sleep_ms);
            }
        }
    }

    application_shutdown();
}

void application_shutdown() {
    if (!application_state) {
        return;
    }

    CORE_INFO("Starting application shutdown...");

    // Call client shutdown if provided
    if (application_state->client_state->shutdown) {
        CORE_DEBUG("Shutting down client...");

        application_state->client_state->shutdown(
            application_state->client_state);

        CORE_DEBUG("Client shutdown complete.");
    }

    CORE_DEBUG("Shutting down UI subsystem...");
    ui_shutdown();
    CORE_DEBUG("UI shutdown complete.");

    CORE_DEBUG("Shutting down assets subsystem...");
    assets_shutdown();
    CORE_DEBUG("Assets shutdown complete.");

    CORE_DEBUG("Shutting down renderer subsystem...");
    renderer_shutdown();
    CORE_DEBUG("Renderer shutdown complete.");

    CORE_DEBUG("Shutting down platform subsystem...");
    platform_shutdown();
    CORE_DEBUG("Platform shutdown complete.");

    CORE_INFO("All subsystems shut down correctly.");

    CORE_DEBUG("Shutting down logging subsystem...");
    log_shutdown();
    CORE_DEBUG("Logger shutdown complete.");

    // Free application state
    memory_deallocate(application_state,
        sizeof(App_State),
        Memory_Tag::APPLICATION);

    application_state = nullptr;
}
