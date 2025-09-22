#pragma once

#include "defines.hpp"

// Forward declarations
struct UI_State;

/**
 * Initialize the dockspace system
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_dockspace_initialize();

/**
 * Shutdown the dockspace system
 */
PROMETHEUS_API void ui_dockspace_shutdown();

/**
 * Begin dockspace rendering
 * Should be called at the start of the main UI render loop
 * @param user_data - UI state pointer
 */
PROMETHEUS_API void ui_dockspace_begin(void* user_data);

/**
 * End dockspace rendering
 * Should be called at the end of the main UI render loop
 */
PROMETHEUS_API void ui_dockspace_end();

/**
 * Render dockspace component
 * @param user_data - UI state pointer
 */
PROMETHEUS_API void ui_dockspace_render(void* user_data);


