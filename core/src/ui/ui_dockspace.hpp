#pragma once

#include "defines.hpp"

// Forward declarations
struct UI_State;

/**
 * ImGui Dockspace Component
 * Provides main application docking area for flexible UI layouts
 * Enables window docking and tabbed interfaces
 */

struct UI_Dockspace_Config {
    b8 enable_docking;
    b8 enable_viewports;
    b8 show_menubar;
    b8 fullscreen_dockspace;
    b8 no_titlebar;
    b8 no_collapse;
    b8 no_resize;
    b8 no_move;
    b8 no_background;
    f32 menubar_height;
    unsigned int window_flags;
    unsigned int dockspace_flags;
};

/**
 * Initialize the dockspace system
 * @param config - dockspace configuration
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_dockspace_initialize(const UI_Dockspace_Config* config);

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

/**
 * Check if dockspace is enabled
 * @return true if dockspace is active
 */
PROMETHEUS_API b8 ui_dockspace_is_enabled();

/**
 * Set dockspace enabled state
 * @param enabled - true to enable, false to disable
 */
PROMETHEUS_API void ui_dockspace_set_enabled(b8 enabled);

/**
 * Get dockspace ID for manual docking operations
 * @return ImGui dockspace ID
 */
PROMETHEUS_API unsigned int ui_dockspace_get_id();

/**
 * Reset dockspace layout to default
 */
PROMETHEUS_API void ui_dockspace_reset_layout();

/**
 * Set whether dockspace should show menubar
 * @param show_menubar - true to show menubar, false to hide
 */
PROMETHEUS_API void ui_dockspace_set_show_menubar(b8 show_menubar);