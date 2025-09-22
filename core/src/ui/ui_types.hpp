#pragma once

#include "defines.hpp"

// UI Layer callback types
typedef void (*PFN_layer_on_render)(void* user_data);
typedef void (*PFN_layer_on_attach)(void* user_data);
typedef void (*PFN_layer_on_detach)(void* user_data);

// Menu callback
typedef void (*PFN_menu_callback)(void* user_data);

// UI Component definition
struct UI_Layer {
    const char* name;
    PFN_layer_on_render on_render;
    PFN_layer_on_attach on_attach;    // Optional - can be nullptr
    PFN_layer_on_detach on_detach;    // Optional - can be nullptr
    void* component_state;            // Component-specific data
};
