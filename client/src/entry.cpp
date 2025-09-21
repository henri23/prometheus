#include <entry.hpp>
#include "ui/client_ui.hpp"
#include "core/logger.hpp"

// Client-specific state structure
struct Client_App_State {
    // Add any client-specific state here later
    b8 initialized;
};

// Client state instance
static Client_App_State client_app_state = {};

// Client lifecycle callback implementations
b8 client_initialize(Client_State* client_state) {
    // Initialize client UI system - this happens after core UI is initialized
    if (!client_ui_initialize()) {
        CLIENT_ERROR("Failed to initialize client UI system");
        return false;
    }

    // Set up client state
    client_app_state.initialized = true;
    client_state->state = &client_app_state;
    CLIENT_INFO("Client initialized.");

    return true;
}

b8 client_update(Client_State* client_state, f32 delta_time) {
    // Client update logic can go here
    return true;
}

b8 client_render(Client_State* client_state, f32 delta_time) {
    // Client render logic can go here
    return true;
}

void client_on_resize(Client_State* client_state, u32 width, u32 height) {
    // Handle resize events
}

void client_shutdown(Client_State* client_state) {
    // Clean up client UI system
    client_ui_shutdown();
	CLIENT_INFO("Client shutdown complete.")

    // Clean up client state
    client_app_state.initialized = false;
}

// Main client initialization function called by core
b8 create_client(Client_State* client_state) {
    // Set up client configuration
    client_state->config.name = "Prometheus Engine";
    client_state->config.width = 1280;
    client_state->config.height = 720;
    client_state->config.icon_path = nullptr;
    client_state->config.window_resizable = true;
    client_state->config.custom_titlebar = true;
    client_state->config.use_dockspace = true;
    client_state->config.center_window = true;

    // Set up lifecycle callbacks
    client_state->initialize = client_initialize;
    client_state->update = client_update;
    client_state->render = client_render;
    client_state->on_resize = client_on_resize;
    client_state->shutdown = client_shutdown;

    // Initialize state pointers
    client_state->state = nullptr;
    client_state->application_state = nullptr;

    return true;
}
