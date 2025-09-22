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

struct Internal_App_State {
    Client* client;
    b8 is_running;
    b8 is_suspended;
    Platform_State plat_state;
};

// Internal pointer to application state for easy access
internal_variable Internal_App_State* internal_state = nullptr;

b8 application_init(Client* client_state) {
    RUNTIME_ASSERT_MSG(client_state, "Client state cannot be null");

    // Protect against multiple initialization
    if (client_state->internal_app_state != nullptr) {
        CORE_ERROR("Application already initialized");
        return false;
    }

    // Allocate application state
    client_state->internal_app_state =
        memory_allocate(sizeof(Internal_App_State), Memory_Tag::APPLICATION);

    internal_state =
        static_cast<Internal_App_State*>(client_state->internal_app_state);

    internal_state->client = client_state;

    if (!log_init()) {
        CORE_FATAL("Failed to initialize log subsystem");
        return false;
    }

    if (!platform_startup(&internal_state->plat_state,
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

	// UI_Theme theme, 
	// Auto_Array<UI_Layer>* layers,
	// PFN_menu_callback menu_callback,
	// const char* app_name) {
    // Initialize UI with configuration from client
    if (!ui_initialize(
            UI_Theme::CATPPUCCIN_MOCHA,
			&client_state->layers,
			client_state->menu_callback,
            client_state->config.name)) {
        CORE_FATAL("Failed to initialize UI subsystem");
        return false;
    }

    // Register UI event callback with platform
    platform_set_event_callback(ui_process_event);

    internal_state->is_running = false;
    internal_state->is_suspended = false;

    CORE_INFO("All subsystems initialized correctly.");

    CORE_DEBUG(
        memory_get_current_usage()); // WARN: Memory leak because the heap
                                     // allocated string must be deallocated

    return true;
}

void application_run() {
    if (!internal_state) {
        CORE_FATAL("Application not initialized");
        return;
    }

    internal_state->is_running = true;

    // Call client initialize if provided
    if (internal_state->client->initialize) {
        if (!internal_state->client->initialize(
                internal_state->client)) {
            CORE_ERROR("Client initialization failed");
            return;
        }
    }

    // Frame rate limiting variables
    f64 frame_start_time = platform_get_absolute_time();

    while (internal_state->is_running) {
        f64 current_time = platform_get_absolute_time();
        f64 delta_time = current_time - frame_start_time;
        frame_start_time = current_time;

        // Process platform events (will forward to UI via callback)
        if (!platform_message_pump()) {
            internal_state->is_running = false;
        }

        // Call client update if provided
        if (internal_state->client->update) {
            if (!internal_state->client->update(
                    internal_state->client,
                    delta_time)) {
                internal_state->is_running = false;
            }
        }

        // Render frame if not suspended
        if (!internal_state->is_suspended) {
            // Start new UI frame
            ui_begin_frame();

            // Call client render if provided
            if (internal_state->client->render) {
                internal_state->client->render(
                    internal_state->client,
                    delta_time);
            }

            // Render the frame
            if (!renderer_draw_frame(ui_render())) {
                internal_state->is_running = false;
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
    if (!internal_state) {
        return;
    }

    CORE_INFO("Starting application shutdown...");

    // Call client shutdown if provided
    if (internal_state->client->shutdown) {
        CORE_DEBUG("Shutting down client...");

        internal_state->client->shutdown(
            internal_state->client);

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
    memory_deallocate(internal_state,
        sizeof(Internal_App_State),
        Memory_Tag::APPLICATION);

    internal_state = nullptr;
}
