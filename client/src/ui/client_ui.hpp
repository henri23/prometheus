#pragma once

#include "defines.hpp"

// Client UI initialization
b8 client_ui_initialize();
void client_ui_shutdown();

// Client component implementations
void client_ui_render_prometheus_window(void* user_data);

// Client menu callback implementation
void client_ui_render_menus(void* user_data);