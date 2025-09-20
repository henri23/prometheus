#pragma once

#include "defines.hpp"

// Forward declaration for client UI config
typedef struct Client_UI_Config Client_UI_Config;

struct App_Config {
    const char* name;
    u32 width;
    u32 height;
    const char* icon_path;
    b8 window_resizable;
    b8 custom_titlebar;
    b8 use_dockspace;
    b8 center_window;
};

// Initialize application with configuration
PROMETHEUS_API b8 application_init(const App_Config* config);

// Run the main application loop
PROMETHEUS_API void application_run();

// Shutdown and cleanup application
void application_shutdown();

// Get current application configuration
PROMETHEUS_API const App_Config* application_get_config();

// Default configuration for easy setup
App_Config application_get_default_config();

// Set client UI configuration
PROMETHEUS_API b8 application_set_client_ui_config(Client_UI_Config* client_config);
