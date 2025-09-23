#include "ui/client_ui.hpp"
#include "app_viewport_layer.hpp"

// Interfaces from core library
#include <core/logger.hpp>
#include <entry.hpp>
#include <events/events.hpp>
#include <input/input_codes.hpp>
#include <memory/memory.hpp>

// Client-specific state structure
struct Frontend_State {
    // Add any client-specific state here later
    b8 initialized;
};

// Memory debug callback function
b8 client_memory_debug_callback(const Event* event) {
    if (event->key.key_code == Key_Code::M && !event->key.repeat) {
        u64 allocation_count = memory_get_allocations_count();
        CLIENT_INFO("Current memory allocations: %llu", allocation_count);
    }
    return false; // Don't consume, let other callbacks process
}

// Client lifecycle callback implementations
b8 client_initialize(Client* client_state) {

    // Register memory debug event listener - press 'M' to show allocation count
    events_register_callback(Event_Type::KEY_PRESSED, client_memory_debug_callback, Event_Priority::LOW);

    // Initialize viewport layer
    if (!app_viewport_layer_initialize()) {
        CLIENT_ERROR("Failed to initialize viewport layer");
        return false;
    }

    CLIENT_INFO("Client initialized.");

    return true;
}

b8 client_update(Client* client_state, f32 delta_time) {
    // Client update logic can go here
    return true;
}

b8 client_render(Client* client_state, f32 delta_time) {
    // Client render logic can go here
    return true;
}

void client_on_resize(Client* client_state, u32 width, u32 height) {
    // Handle resize events
}

void client_shutdown(Client* client_state) {
    // Shutdown viewport layer
    app_viewport_layer_shutdown();

    // Clean up frontend state
    memory_deallocate(client_state->state,
        sizeof(Frontend_State),
        Memory_Tag::CLIENT);

    CLIENT_INFO("Client shutdown complete.")
}

// Main client initialization function called by core
b8 create_client(Client* client_state) {
    // Set up client configuration
    client_state->config.name = "Prometheus EDA";
    client_state->config.width = 1280;
    client_state->config.height = 720;
    client_state->config.theme = UI_Theme::CATPPUCCIN_MOCHA;

    // Set up lifecycle callbacks
    client_state->initialize = client_initialize;
    client_state->update = client_update;
    client_state->render = client_render;
    client_state->on_resize = client_on_resize;
    client_state->shutdown = client_shutdown;

    // Initialize state pointers
    client_state->state =
        memory_allocate(sizeof(Frontend_State), Memory_Tag::CLIENT);

    client_state->layers.push_back({
        .name = "prometheus_window",
            .on_render = client_ui_render_prometheus_window,
            .on_attach = nullptr,
            .on_detach = nullptr,
            .component_state = nullptr
    });

    client_state->layers.push_back({
        .name = "viewport_layer",
            .on_render = app_viewport_layer_render,
            .on_attach = nullptr,
            .on_detach = nullptr,
            .component_state = nullptr
    });

    client_state->menu_callback = client_ui_render_menus;

    return true;
}
