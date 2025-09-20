#pragma once

#include "defines.hpp"

// Forward declarations
struct UI_State;

// Client UI component callback types
typedef void (*Client_UI_RenderCallback)(void* user_data);
typedef void (*Client_UI_AttachCallback)(void* user_data);
typedef void (*Client_UI_DetachCallback)(void* user_data);
typedef void (*Client_MenuCallback)(void* user_data);

// Client UI component definition
typedef struct Client_UI_Component {
    const char* name;
    Client_UI_RenderCallback on_render;
    Client_UI_AttachCallback on_attach;    // Optional - can be nullptr
    Client_UI_DetachCallback on_detach;    // Optional - can be nullptr
    void* user_data;                       // Client-specific data
    b8 is_active;
} Client_UI_Component;

// Client UI configuration passed to core
typedef struct Client_UI_Config {
    Client_UI_Component* components;       // Array of client components
    u32 component_count;                   // Number of components
    Client_MenuCallback menu_callback;    // Optional menu callback
    void* menu_user_data;                  // Data for menu callback
} Client_UI_Config;

// Client UI initialization and component definitions
b8 client_ui_initialize();
void client_ui_shutdown();

// Get the client UI configuration for core
Client_UI_Config* client_ui_get_config();

// Client component implementations
void client_ui_render_prometheus_window(void* user_data);

// Client menu callback implementation
void client_ui_render_menus(void* user_data);