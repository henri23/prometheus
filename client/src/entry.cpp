#include "ui/client_ui.hpp"

// Interfaces from core library
#include <core/logger.hpp>
#include <entry.hpp>
#include <memory/memory.hpp>

// Client-specific state structure
struct Frontend_State {
    // Add any client-specific state here later
    b8 initialized;
};

// Client lifecycle callback implementations
b8 client_initialize(Client* client_state) {

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

    client_state->menu_callback = client_ui_render_menus;

    return true;
}
